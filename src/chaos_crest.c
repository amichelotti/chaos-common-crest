/** chaos_crest.c
 * @author Andrea Michelotti
 * @version 0.1
 * @date 26/2/2015
 * Simple apis to access chaos using http rest
 */

#include "chaos_crest.h"
#include <stdlib.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <netdb.h>
#include <http_lib.h>
#define MAXDIGITS 16
#define MAXBUFFER 8192

#ifdef CREST_DEBUG
#define DPRINT(x,ARGS...) printf(x, ##ARGS)
#else
#define DPRINT(x,...)
#endif


typedef struct _ds{
    const char *name;
    const char *desc;
    int        type;
    int        size;
    char*      format;
    char*      data; //null terminated string containing the data
} ds_t;
typedef struct _cu {
    const char*name;
    ds_t*inds;
    int nin;
    ds_t*outds;
    int nout;
} cu_t;



typedef struct _chaos_crest_handle{
    char* wan_url;
    int sock_fd;
    char*hostname;
    http_handle_t http;

    cu_t *cus;
    int ncus;
    struct sockaddr_in sin;
    int connected;
    uint64_t tot_registration_time;
    uint64_t tot_push_time;
    uint32_t npush;
    uint32_t nreg;
  uint32_t min_push;
  uint32_t max_push;
} _chaos_crest_handle_t;


static int http_post(chaos_crest_handle_t h,char*api,char*trx_buffer,int tsizeb,char*rx_buffer,int rsizeb){
  _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
  int ret;
  if(rx_buffer==0){
    ret = http_perform_request(p->http,"POST",p->hostname, "chaos_crest",api, "application/json",trx_buffer);
    if(ret>0){
      return 0;
    }
    return ret;
  } else{
    ret = http_request(p->http,"POST",p->hostname, "chaos_crest",api, "application/json",trx_buffer, rx_buffer, rsizeb);
  }
  if(ret==200)
    return 0;
  
  return ret;
}

static const char* typeToString(int type){
    if(type==TYPE_INT32){
             return "int32";
    } else if(type==TYPE_INT64){
             return "int64";
    } else if(type==TYPE_DOUBLE){
             return "double";
    } else if(type==TYPE_STRING){
             return "string";
             
    } else {
             printf("## unsupported type(for the moment)\n");
             assert(0);
    }
    return "";
}

static const char* typeToFormat(int type){
 if(type==TYPE_INT32){
             return "d";
    } else if(type==TYPE_INT64){
             return "lld";
    } else if(type==TYPE_DOUBLE){
             return "f";
    } else if(type==TYPE_STRING){
             return "s";
             
    } else {
             printf("## unsupported type(for the moment)\n");
             assert(0);
    }
    return "";
}
static unsigned long long getEpoch(){
    struct timeval mytime;
    uint64_t ret;
    gettimeofday(&mytime,NULL);
    ret = mytime.tv_sec*1000 + mytime.tv_usec/1000;
    return ret;
}
chaos_crest_handle_t chaos_crest_open(const char* chaoswan_url) {
    struct hostent * host_addr;
    
    char host[256];
    int sock;
    char*hostname = host;
    char*sport;
    int port;
    int one=1;
    int opts;
    _chaos_crest_handle_t*h;
    strcpy(host, chaoswan_url);
    sport = strstr(host, ":");

    if (sport == 0) {
        return NULL;
    }
    port = atoi(sport + 1);
    *sport = 0;
    sock = socket(AF_INET, SOCK_STREAM, 0);
 
    if (sock < 0) 
        return 0;
    /*    if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one))<0){
      perror("cannot make no delay");
      return 0;
      }*/
    opts = fcntl(sock,F_GETFL);
    opts = opts & (~O_NONBLOCK);

    fcntl(sock, F_SETFL, opts);
    h = (_chaos_crest_handle_t*)malloc(sizeof(_chaos_crest_handle_t));
    h->wan_url=strdup(chaoswan_url);
    h->sock_fd=sock;
    h->ncus=0;
    h->cus=0;
    h->connected=0;
    bzero(&h->sin, sizeof (struct sockaddr_in));
    h->sin.sin_family = AF_INET;
    h->sin.sin_port = htons((unsigned short) port);
    host_addr = gethostbyname(hostname);
    if (host_addr == NULL) {
        printf("## Unable to locate host %s\n",hostname);
        return 0;
    }
    bcopy(host_addr->h_addr, &h->sin.sin_addr.s_addr, host_addr->h_length);
    h->hostname=strdup(host_addr->h_name);
    h->http=http_client_init(sock);
    if(h->http==0){
        chaos_crest_close(h);
        printf("## Unable to initialize http resources\n");

        return 0;
    }
    h->min_push=0xfffffff;
    h->max_push=0;
    DPRINT("* open socket %d \"%s\"->\"%s\":\"%d\"\n",h->sock_fd,h->hostname,hostname,port);
    
    return h;
}

int chaos_crest_connect(chaos_crest_handle_t h) {
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    int ret;
    p->connected=0;
    
    if(p->sock_fd<=0){
        printf("## not opened failed");
        return -2;
    }
    if ((ret=connect(p->sock_fd, (const struct sockaddr *) &p->sin, sizeof (struct sockaddr_in))) != 0) {
      printf("## connect failed, ret=%d",ret) ;
        close(p->sock_fd);
        p->sock_fd=0;
        return -101;
    }
    p->connected=1;
    return 0;
}

uint32_t chaos_crest_add_cu(chaos_crest_handle_t h,const char*name,chaos_ds_t* dsin,int dsitems){
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    int cnt;
    int ndsin=0,ndsout=0;
    int cnt_in=0,cnt_out=0;

    p->cus=(cu_t*)realloc(p->cus,(p->ncus+1)*sizeof(cu_t));
    if(p->cus==NULL){
        printf("## cannot allocate memory\n");
        return -4;
    }
    
    p->cus[p->ncus].name=strdup(name);
    p->cus[p->ncus].inds=0;
    p->cus[p->ncus].outds=0;
    for(cnt=0;cnt<dsitems;cnt++){
        ndsin+=((dsin[cnt].dir==DIR_INPUT )||(dsin[cnt].dir==DIR_IO))?1:0;
        ndsout+=((dsin[cnt].dir==DIR_OUTPUT )||(dsin[cnt].dir==DIR_IO))?1:0;

    }
    if(ndsin>0){
      p->cus[p->ncus].inds=(ds_t*)calloc(ndsin,sizeof(ds_t));
      p->cus[p->ncus].nin=ndsin;
    }
  
    if(ndsout>0){
      p->cus[p->ncus].outds=(ds_t*)calloc(ndsout,sizeof(ds_t));
      p->cus[p->ncus].nout=ndsout;

    }    
    for(cnt=0;cnt<dsitems;cnt++){
        if(((dsin[cnt].dir==DIR_INPUT )||(dsin[cnt].dir==DIR_IO))){
	  p->cus[p->ncus].inds[cnt_in].name= strdup(dsin[cnt].name);
	  //	  printf("name:%s 0x%x\n",p->cus[p->ncus].inds[cnt_in].name,p->cus[p->ncus].inds[cnt_in].name);

	  p->cus[p->ncus].inds[cnt_in].desc= strdup(dsin[cnt].desc);
	  p->cus[p->ncus].inds[cnt_in].type= dsin[cnt].type;
	  p->cus[p->ncus].inds[cnt_in].size= dsin[cnt].size;
	  cnt_in++;
        } 
	if(((dsin[cnt].dir==DIR_OUTPUT )||(dsin[cnt].dir==DIR_IO))){
            const char* stype;
            const char*sformat;
            char stringa[256];
            p->cus[p->ncus].outds[cnt_out].name= strdup(dsin[cnt].name);
            p->cus[p->ncus].outds[cnt_out].desc= strdup(dsin[cnt].desc);
            p->cus[p->ncus].outds[cnt_out].type= dsin[cnt].type;
            p->cus[p->ncus].outds[cnt_out].size= dsin[cnt].size;
            stype=typeToString(dsin[cnt].type);
            sformat=typeToFormat(dsin[cnt].type);
            snprintf(stringa,sizeof(stringa),"\"%s\":\"%s:%%%s\"",dsin[cnt].name,stype,sformat);
            p->cus[p->ncus].outds[cnt_out].format=strdup(stringa);
	    p->cus[p->ncus].outds[cnt_out].size= (strlen(stringa)+1+dsin[cnt].size+MAXDIGITS) ;
	    DPRINT("allocating %d bytes\n",p->cus[p->ncus].outds[cnt_out].size);
            p->cus[p->ncus].outds[cnt_out].data= calloc(1,p->cus[p->ncus].outds[cnt_out].size);
            if((p->cus[p->ncus].outds[cnt_out].data==NULL) || (p->cus[p->ncus].outds[cnt_out].format==NULL) ){
                printf("## cannot allocate memory for attribute data\n");
                return -6;
            }
	    cnt_out++;
	}
    }
    
    p->ncus++;
    return p->ncus; // return the UID
}


static int update_attribute(cu_t *cu,int attr,void*data){
    ds_t*p=cu->outds + attr;
    if((attr >= cu->nout)  || (attr<0)){
        printf("## invalid attribute index for output, max is:%d\n",cu->nout-1);
        return -7;
    }
    if(p->type==TYPE_INT32){
        snprintf(p->data,p->size,p->format,*(int32_t*)data);
    } else if(p->type==TYPE_INT64){
       snprintf(p->data,p->size,p->format,*(int64_t*)data);
    } else if(p->type==TYPE_DOUBLE){
       snprintf(p->data,p->size,p->format,*(double*)data);
    }else if(p->type==TYPE_STRING){
       snprintf(p->data,p->size,p->format,(char*)data);
    }
    DPRINT("updating [%d]\"%s\" format \"%s\" size:%d type:%d (0x%x)\n",attr,p->data,p->format,p->size,p->type,data);
    return 0;
}

int chaos_crest_update(chaos_crest_handle_t h,uint32_t cu_uid,int attr_pos,void*data){
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    cu_t *cu;
    if((cu_uid>p->ncus) || (cu_uid<=0)){
        printf("## bad Id %d",cu_uid);
        return -8; 
    }
    cu=p->cus + (cu_uid-1);
    return update_attribute(cu,attr_pos,data);
}


int chaos_crest_update_by_name(chaos_crest_handle_t h,uint32_t cu_uid,char* attr_name,void*data){
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    cu_t *cu;
    int cnt;
    if((cu_uid>p->ncus) || (cu_uid<=0)){
        printf("## bad Id %d",cu_uid);
        return -8;
    }
    cu=p->cus + (cu_uid-1);

    for(cnt=0;cnt<cu->nout;cnt++){
        if(!strcmp(cu->outds[cnt].name,attr_name)){
             return update_attribute(cu,cnt,data);
        }
    }
    return -9;// attribute not found
}

int chaos_crest_close(chaos_crest_handle_t h){
    int cnt;
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    http_client_deinit(p->http);
    cu_t* cus= p->cus;
    close(p->sock_fd);

    for(cnt=0;cnt<p->ncus;cnt++){
      int cntt;
      if(cus[cnt].name){
	free((void*)cus[cnt].name);
	cus[cnt].name=0;
      }
     

      for(cntt=0;cntt<cus[cnt].nout;cntt++){
	if(cus[cnt].outds[cntt].name){
	  free((void*)cus[cnt].outds[cntt].name);
	  cus[cnt].outds[cntt].name=0;
	}
	if(cus[cnt].outds[cntt].desc){
	  free((void*)cus[cnt].outds[cntt].desc);
	  cus[cnt].outds[cntt].desc=0;
	}
	if(cus[cnt].outds[cntt].format){
	  free((void*)cus[cnt].outds[cntt].format);
	  cus[cnt].outds[cntt].format=0;
	}
	  
	if(cus[cnt].outds[cntt].data){
	  free((void*)cus[cnt].outds[cntt].data);
	  cus[cnt].outds[cntt].data=0;
	}
      }

      for(cntt=0;cntt<cus[cnt].nin;cntt++){
	if(cus[cnt].inds[cntt].name){
	  free((void*)cus[cnt].inds[cntt].name);
	  cus[cnt].inds[cntt].name=0;
	}
	if(cus[cnt].inds[cntt].desc){
	  free((void*)cus[cnt].inds[cntt].desc);
	  cus[cnt].inds[cntt].desc=0;
	}
      }
      free((void*)cus[cnt].inds);
      free((void*)cus[cnt].outds);
      cus[cnt].inds=0;
      cus[cnt].outds=0;
    }
    free(p->cus);
    p->cus=0;
    free(p->hostname);
    free(p->wan_url);
    free(h);
    return 0;
}

static int dump_attribute_desc(ds_t *attr,char*dir,char*buffer,int size,int last){
    
    return snprintf(buffer,size,"{\"ds_attr_name\":\"%s\",\"ds_attr_desc\":\"%s\",\"ds_attr_type\":\"%s\",\"ds_attr_dir\":\"%s\"}%s",
            attr->name,attr->desc,typeToString(attr->type),dir,last?"":",");
}

static int dump_attribute_value(ds_t *attr,char*buffer,int size,int last){
    
    return snprintf(buffer,size,"%s%s",
            attr->data,last?"":",");
}
static int register_cu(chaos_crest_handle_t h,uint32_t cu_uid,char*buffer,int size){
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    cu_t* cu;
    int cnt;
    char*pnt;
    char url[256];
    char buffer_rx[MAXBUFFER];
    int csize=0;
    unsigned long long ts;
    int ret;
    if((cu_uid>p->ncus) || (cu_uid<=0)){
        printf("## bad Id %d",cu_uid);
        return -8;
    }
    cu=p->cus + (cu_uid-1);
    DPRINT("registering CU \"%s\" UID:%d IN:%d, OUT:%d\n",cu->name,cu_uid,cu->nin,cu->nout);
    ts=getEpoch();
    snprintf(buffer,size,"{\"ds_attr_dom\":\"%s\",\"ds_timestamp\":%llu,\"ds_desc\":[",cu->name,ts);
    for(cnt=0;cnt<cu->nin;cnt++){
        csize=strlen(buffer);
        pnt=buffer+csize;
        dump_attribute_desc(cu->inds+cnt,"input",pnt,size-csize,(((cnt+1)==cu->nin))&&(cu->nout==0));
    }

    for(cnt=0;cnt<cu->nout;cnt++){
        csize=strlen(buffer);
        pnt=buffer+csize;
        dump_attribute_desc(cu->outds+cnt,"output",pnt,size-csize,((cnt+1)==cu->nout));
       
    }
    strcat(buffer,"]}");
    snprintf(url,sizeof(url),"/api/v1/producer/register/%s",cu->name);
    *buffer_rx=0;
    if((ret=http_post(h,url,buffer,strlen(buffer),buffer_rx,sizeof(buffer_rx)))==0){
        p->tot_registration_time+= (getEpoch() -ts); 
        p->nreg++;
	DPRINT("server returned:'%s'\n",buffer_rx);
	return 0;
    }
    printf("post failed to:\"%s\" ret:%d,server answer:'%s'\n",url,ret,buffer_rx);
    return -9; // registration failure
}


static int push_cu(chaos_crest_handle_t h,uint32_t cu_uid,char*buffer,int size){
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    cu_t* cu;
    int cnt;
    char*pnt;
    char url[256];
    char buffer_rx[MAXBUFFER];

    int csize;
    unsigned long long ts;
    if((cu_uid>p->ncus) || (cu_uid<=0)){
        printf("## bad Id %d",cu_uid);
        return -8;
    }
    
    cu=p->cus + (cu_uid-1);
    ts =getEpoch();
    snprintf(buffer,size,"{\"dpck_ts\":%llu%s",getEpoch(),cu->nout>0?",":"");
    for(cnt=0;cnt<cu->nout;cnt++){
        csize=strlen(buffer);
        pnt=buffer+csize;
        dump_attribute_value(cu->outds+cnt,pnt,size-csize,((cnt+1)==cu->nout));
    }
    strcat(buffer,"}");
    
    snprintf(url,sizeof(url),"/api/v1/producer/insert/%s",cu->name);
    
    if(http_post(h,url,buffer,strlen(buffer),0,0/*buffer_rx,sizeof(buffer_rx)*/)==0){
      uint32_t t=(getEpoch() -ts); 
      p->tot_push_time+= t;
      p->max_push=(t>p->max_push)?t:p->max_push;
      p->min_push=(t<p->min_push)&& (t>0)?t:p->min_push;
      p->npush++;
        return 0;
    }
    
    return -9; // registration failure
}
int chaos_crest_register(chaos_crest_handle_t h,uint32_t cu_cuid){
   _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    cu_t* cu=p->cus;
    int ret;
    char buffer[MAXBUFFER];
    if(cu_cuid == 0){
        int cnt;
        for(cnt=0;cnt<p->ncus;cnt++){
            if((ret=register_cu(h,cnt+1,buffer,sizeof(buffer)))!=0){
                return ret;
            }
        }
        return 0;
    } 

    return register_cu(h,cu_cuid,buffer,sizeof(buffer));
}

int chaos_crest_push(chaos_crest_handle_t h,uint32_t cu_uid){
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    cu_t* cu=p->cus;
    int ret;
    char buffer[MAXBUFFER];
     if(cu_uid == 0){
        int cnt;
        for(cnt=0;cnt<p->ncus;cnt++){
            if((ret=push_cu(h,cnt+1,buffer,sizeof(buffer)))!=0){
                return ret;
            }
        }
        return 0;
    } 

    return push_cu(h,cu_uid,buffer,sizeof(buffer));
}


float chaos_crest_push_time(chaos_crest_handle_t h,uint32_t*max,uint32_t *min){
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    if(max){
      *max = p->max_push;
    }
    if(min){
      *min = p->min_push;
    }

    if(p->npush>0){
        return 1.0*(float)p->tot_push_time/p->npush;
    }
    return 0;
         
}

float chaos_crest_reg_time(chaos_crest_handle_t h){
    _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
    if(p->nreg>0){
        return 1.0*(float)p->tot_registration_time/p->nreg;
    }
    return 0;
         
}


int chaos_crest_cu_cmd(chaos_crest_handle_t h,const char*cuname,const char*cmd,const char* args){
  char command[1024];
  int ret;
  _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
  if(cmd==0 || cuname==0)
    return -1;

  if(args){
    sprintf(command,"/CU?dev=%s&cmd=%s&parm=%s",cuname,cmd,args);
  } else {
    sprintf(command,"/CU?dev=%s&cmd=%s",cuname,cmd);
  }
  ret = http_request(p->http,"GET",p->wan_url, "chaos_crest_cu_cmd",command, "application/text","", 0, 0);
  if(ret==200)
    return 0;
  
  return ret;
}

uint64_t chaos_crest_cu_get(chaos_crest_handle_t h,const char*cuname,char*output,int maxsize){
  uint64_t rett=0;
  
  char cmd[256];
  _chaos_crest_handle_t*p=(_chaos_crest_handle_t*)h;
  int ret;
  if(output==0 || cuname==0)
    return -1;
  sprintf(cmd,"/CU?dev=%s&cmd=status",cuname);
  
  ret = http_request(p->http,"GET",p->wan_url, "chaos_crest_cu_get",cmd, "html/text",0, output, maxsize);
  if(ret<0){
    printf("## error getting cu %s\n",cuname);
  }

  
  
  if(ret==200){
    char *pnt=strstr(output,"\"dpck_ts\" : { \"$numberLong\" : \"");
    if(pnt){
      rett=atoll(pnt);
      printf("DECODIFIED %s %llu\n",pnt,rett);
      return rett;
      
    }
    return -1;
  }
  
  return 0;
}

uint64_t chaos_crest_cu_get_channel(chaos_crest_handle_t h,const char*cuname,const char*channame,char*output,int maxsize){
  char buf[MAXBUFFER];
  uint64_t ret;
  ret=  chaos_crest_cu_get(h,cuname,buf,sizeof(buf));
  if(ret>0){
    char search[256];
    char *pnt,*pntt;
    sprintf(search,"\"%s\":\"",channame);
    pnt=strstr(buf,search);
    if(pnt){
      pnt+=strlen(search);
      pntt=strchr(pnt,'\"');
      if(pntt)*pntt=0;
      strncpy(output,pnt,maxsize);
      return ret;
    }
  }
  return ret;
}


