/** chaos_crest.c
 * @author Andrea Michelotti
 * @version 0.1
 * @date 26/2/2015
 * Simple apis to access chaos using http rest
 */
 #include <stdlib.h>
#include "b64.h"
#include "chaos_crest.h"

#include <signal.h>
#include <inttypes.h>

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
#define MOONGOOSE
#ifndef MOONGOOSE
#include <http_lib.h>
#else
#include "mongoose.h"
#endif

#define MAXDIGITS 16
#define MAXBUFFER 8192
//#define CREST_DEBUG
#ifdef CREST_DEBUG
#define DPRINT(x, ARGS...) printf(x, ##ARGS)

#else
#define DPRINT(x, ...)
#endif

typedef struct _ds
{
  const char *name;
  const char *desc;
  int type;
  int size;
  char *format;
  char *data; //null terminated string containing the data
  int        alloc_size;

} ds_t;
typedef struct _cu
{
  const char *name;
  ds_t *inds;
  int nin;
  ds_t *outds;
  int nout;
  int maxbufsize;
} cu_t;

typedef struct _chaos_crest_handle
{

  char *wan_url;

  int sock_fd;
  char *hostname;
#ifndef MOONGOOSE
  http_handle_t http;
#else
  struct mg_mgr mgr;
#endif
  cu_t *cus;
  int ncus;
  struct sockaddr_in sin;
  int connected;
  uint64_t tot_registration_time;
  uint64_t tot_push_time;
  uint64_t npush;
  uint32_t nreg;
  uint32_t min_push;
  uint32_t max_push;

} _chaos_crest_handle_t;

#ifdef MOONGOOSE
static int s_exit_flag = 0;

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
  struct http_message *hm = (struct http_message *)ev_data;
  int connect_status;

  switch (ev)
  {
  case MG_EV_CONNECT:
    connect_status = *(int *)ev_data;
    if (connect_status != 0)
    {
      // printf("Error connecting to %s: %s\n", hm->uri, strerror(connect_status));
      s_exit_flag = -1;
    }
    break;
  case MG_EV_HTTP_REPLY:
    // printf("Got reply:\n%.*s\n%d\n", (int) hm->body.len, hm->body.p,hm->resp_code);
    nc->flags |= MG_F_SEND_AND_CLOSE;
    s_exit_flag = hm->resp_code;
    break;
  case MG_EV_CLOSE:
    if (s_exit_flag == 0)
    {
      //    printf("Server closed connection\n");
      s_exit_flag = 1;
    };
    break;
  default:
    break;
  }
}
#endif

int http_post(chaos_crest_handle_t h, const char *api, const char *trx_buffer, int tsizeb, char *rx_buffer, int rsizeb)
{
  _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
  int ret;
  struct mg_connection *nc;
  char s_url[256];
  snprintf(s_url, sizeof(s_url), "%s%s", p->wan_url, api);
  s_exit_flag = 0;
  DPRINT("POSTING:%s\n",trx_buffer);
  nc = mg_connect_http(&p->mgr, ev_handler, s_url, "Content-Type:application/json\r\nConnection:keep-alive\r\n", trx_buffer);
  while (s_exit_flag == 0)
  {
    mg_mgr_poll(&p->mgr, 1);
  }
  ret = (s_exit_flag > 0) ? 0 : s_exit_flag;
  
#ifndef MOONGOOSE

  if (rx_buffer == 0)
  {
    ret = http_perform_request(p->http, "POST", p->hostname, "chaos_crest", api, "application/json", trx_buffer);
    if (ret > 0)
    {
      return 0;
    }
    return ret;
  }
  else
  {
    ret = http_request(p->http, "POST", p->hostname, "chaos_crest", api, "application/json", trx_buffer, rx_buffer, rsizeb);
  }
  if (ret == 200)
    return 0;
#else
#endif
  return ret;
}

int chaos_crest_json_register(chaos_crest_handle_t h, const char *cu_uid, const char *json_ds)
{
  int ret;
  char url[MAXBUFFER];
  char buffer_rx[MAXBUFFER];
  snprintf(url, sizeof(url), "/api/v1/producer/jsonregister/%s", cu_uid);
  if ((ret = http_post(h, url, json_ds, strlen(json_ds), buffer_rx, sizeof(buffer_rx))) == 0)
  {

    DPRINT("server returned:'%s'\n", buffer_rx);
    return 0;
  }
  DPRINT("## json registration failed to:\"%s\" ret:%d,server answer:'%s'\n", url, ret, buffer_rx);
  return ret;
}

int chaos_crest_json_push(chaos_crest_handle_t h, const char *cu_uid, const char *json_ds)
{
  int ret;
  char url[MAXBUFFER];
  char buffer_rx[MAXBUFFER];
  snprintf(url, sizeof(url), "/api/v1/producer/jsoninsert/%s", cu_uid);
  if ((ret = http_post(h, url, json_ds, strlen(json_ds), buffer_rx, sizeof(buffer_rx))) == 0)
  {

    DPRINT("server returned:'%s'\n", buffer_rx);
    return 0;
  }
  DPRINT("##json insert failed to:\"%s\" ret:%d,server answer:'%s'\n", url, ret, buffer_rx);
  return ret;
}
/*
static const char *typeNumberToString(int type)
{
  static char buf[256];
  if (type == CREST_TYPE_INT32)
  {

    return "int32";
  }
  else if (type == CREST_TYPE_INT64)
  {
    return "int64";
  }
  else if (type == CREST_TYPE_DOUBLE)
  {
    return "double";
  }
  else if (type == CREST_TYPE_STRING)
  {
    return "string";
  }
  else
  {
    printf("## unsupported type(for the moment)\n");
    assert(0);
  }
  return "";
}
*/
static const char *typeToString(int type)
{
  if (type == CREST_TYPE_INT32)
  {
    return "int32";
  }
  else if (type == CREST_TYPE_INT64)
  {
    return "int64";
  }
  else if (type == CREST_TYPE_DOUBLE)
  {
    return "double";
  }
  else if (type == CREST_TYPE_STRING)
  {
    return "string";
  }
  else
  {
    printf("## unsupported type(for the moment)\n");
    assert(0);
  }
  return "";
}

static const char *typeToFormat(int type)
{
  if (type == CREST_TYPE_INT32)
  {
    return "%d";
  }
  else if (type == CREST_TYPE_INT64)
  {
    return "{\"$numberLong\":\"%lld\"}";
  }
  else if (type == CREST_TYPE_DOUBLE)
  {
    return "{\"$numberDouble\":\"%lf\"}";
  }
  else if (type == CREST_TYPE_STRING)
  {
    return "\"%s\"";
  }
  else if (type == CREST_TYPE_BINARY)
  {
    return "{\"$binary\":{\"base64\":\"%s\",\"subType\":\"00\"}}}";
  }
  else if ((type & CREST_TYPE_INT32) && (type & CREST_TYPE_VECTOR))
  {
    return "{\"$binary\":{\"base64\":\"%s\",\"subType\":\"84\"}}";
  }
  else if ((type & CREST_TYPE_DOUBLE) && (type & CREST_TYPE_VECTOR))
  {
    return "{\"$binary\":{\"base64\":\"%s\",\"subType\":\"86\"}}";
  }
  else if ((type & CREST_TYPE_INT64) && (type & CREST_TYPE_VECTOR))
  {
    return "{\"$binary\":{\"base64\":\"%s\",\"subType\":\"85\"}}";
  } else if ((type & CREST_TYPE_INT16) && (type & CREST_TYPE_VECTOR))
  {
    return "{\"$binary\":{\"base64\":\"%s\",\"subType\":\"83\"}}";
  }
  else
  {
    printf("## unsupported type(for the moment)\n");
    assert(0);
  }
  return "";
}
static unsigned long long getEpoch()
{
  struct timeval mytime;
  unsigned long long ret;
  gettimeofday(&mytime, NULL);
  ret = (unsigned long long)mytime.tv_sec * 1000 + ((unsigned long long)mytime.tv_usec / 1000);

  return ret;
}
chaos_crest_handle_t chaos_crest_open(const char *chaoswan_url)
{

#ifndef MOONGOOSE

  struct hostent *host_addr;
  char host[256];
  int sock;
  char *hostname = host;
  char *sport;
  int port;
  int opts;
  _chaos_crest_handle_t *h;
  strcpy(host, chaoswan_url);
  sport = strstr(host, ":");

  if (sport == 0)
  {
    return NULL;
  }
  port = atoi(sport + 1);
  *sport = 0;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  signal(SIGPIPE, SIG_IGN);

  if (sock < 0)
    return 0;
  /*    if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one))<0){
      perror("cannot make no delay");
      return 0;
      }*/
  opts = fcntl(sock, F_GETFL);
  opts = opts & (~O_NONBLOCK);

  fcntl(sock, F_SETFL, opts);
  h = (_chaos_crest_handle_t *)calloc(1,sizeof(_chaos_crest_handle_t));
  h->wan_url = strdup(chaoswan_url);
  h->sock_fd = sock;
  h->ncus = 0;
  h->cus = 0;
  h->connected = 0;
  bzero(&h->sin, sizeof(struct sockaddr_in));
  h->sin.sin_family = AF_INET;
  h->sin.sin_port = htons((unsigned short)port);
  host_addr = gethostbyname(hostname);
  if (host_addr == NULL)
  {
    printf("## Unable to locate host %s\n", hostname);
    return 0;
  }
  bcopy(host_addr->h_addr_list[0], &h->sin.sin_addr.s_addr, host_addr->h_length);
  h->hostname = strdup(host_addr->h_name);
  h->http = http_client_init(sock);
  if (h->http == 0)
  {
    chaos_crest_close(h);
    printf("## Unable to initialize http resources\n");

    return 0;
  }
  h->min_push = 0xfffffff;
  h->max_push = 0;
  if (chaos_crest_connect(h) != 0)
  {
    printf("## cannot connect\n");
    chaos_crest_close(h);
    return 0;
  }
  DPRINT("* open socket %d \"%s\"->\"%s\":\"%d\"\n", h->sock_fd, h->hostname, hostname, port);
#else
  _chaos_crest_handle_t *h;
  h = (_chaos_crest_handle_t *)calloc(1,sizeof(_chaos_crest_handle_t));
  h->wan_url = strdup(chaoswan_url);
  mg_mgr_init(&h->mgr, NULL);
#endif

  return h;
}

int chaos_crest_connect(chaos_crest_handle_t h)
{
#ifndef MOONGOOSE

  _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
  int ret;
  p->connected = 0;

  if (p->sock_fd <= 0)
  {
    printf("## not opened failed");
    return -2;
  }
  if ((ret = connect(p->sock_fd, (const struct sockaddr *)&p->sin, sizeof(struct sockaddr_in))) != 0)
  {
    printf("## connect failed, ret=%d", ret);
    close(p->sock_fd);
    p->sock_fd = 0;
    return -101;
  }
  p->connected = 1;
#endif
  return 0;
}

uint32_t chaos_crest_add_cu(chaos_crest_handle_t h, const char *name, chaos_ds_t *dsin, int dsitems)
{
  _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
  int cnt;
  int ndsin = 0, ndsout = 0;
  int cnt_in = 0, cnt_out = 0;

  p->cus = (cu_t *)realloc(p->cus, (p->ncus + 1) * sizeof(cu_t));
  if (p->cus == NULL)
  {
    printf("## cannot allocate memory\n");
    return -4;
  }

  p->cus[p->ncus].name = strdup(name);
  p->cus[p->ncus].inds = 0;
  p->cus[p->ncus].outds = 0;
  p->cus[p->ncus].maxbufsize = 0;
  for (cnt = 0; cnt < dsitems; cnt++)
  {
    ndsin += ((dsin[cnt].dir == CREST_DIR_INPUT) || (dsin[cnt].dir == CREST_DIR_IO)) ? 1 : 0;
    ndsout += ((dsin[cnt].dir == CREST_DIR_OUTPUT) || (dsin[cnt].dir == CREST_DIR_IO)) ? 1 : 0;
  }
  if (ndsin > 0)
  {
    p->cus[p->ncus].inds = (ds_t *)calloc(ndsin, sizeof(ds_t));
    p->cus[p->ncus].nin = ndsin;
  }

  if (ndsout > 0)
  {
    p->cus[p->ncus].outds = (ds_t *)calloc(ndsout, sizeof(ds_t));
    p->cus[p->ncus].nout = ndsout;
  }
  for (cnt = 0; cnt < dsitems; cnt++)
  {
    if (((dsin[cnt].dir == CREST_DIR_INPUT) || (dsin[cnt].dir == CREST_DIR_IO)))
    {
      p->cus[p->ncus].inds[cnt_in].name = strdup(dsin[cnt].name);
      //	  printf("name:%s 0x%x\n",p->cus[p->ncus].inds[cnt_in].name,p->cus[p->ncus].inds[cnt_in].name);

      p->cus[p->ncus].inds[cnt_in].desc = strdup(dsin[cnt].desc);
      p->cus[p->ncus].inds[cnt_in].type = dsin[cnt].type;
      p->cus[p->ncus].inds[cnt_in].size = dsin[cnt].size;
      cnt_in++;
    }
    if (((dsin[cnt].dir == CREST_DIR_OUTPUT) || (dsin[cnt].dir == CREST_DIR_IO)))
    {
      const char *stype;
      const char *sformat;
      char stringa[256];
      p->cus[p->ncus].outds[cnt_out].name = strdup(dsin[cnt].name);
      p->cus[p->ncus].outds[cnt_out].desc = strdup(dsin[cnt].desc);
      p->cus[p->ncus].outds[cnt_out].type = dsin[cnt].type;
      p->cus[p->ncus].outds[cnt_out].size = dsin[cnt].size;
      //stype=typeToString(dsin[cnt].type);
      sformat = typeToFormat(dsin[cnt].type);
      snprintf(stringa, sizeof(stringa), "\"%s\":%s", dsin[cnt].name, sformat);
      p->cus[p->ncus].outds[cnt_out].format = strdup(stringa);
      p->cus[p->ncus].outds[cnt_out].size = (strlen(stringa) + 1 + dsin[cnt].size + MAXDIGITS);
      dsin[cnt].alloc_size = 64;

      if ((dsin[cnt].type == CREST_TYPE_BINARY) || (dsin[cnt].type & CREST_TYPE_VECTOR))
      {
        p->cus[p->ncus].outds[cnt_out].size=dsin[cnt].size;
        dsin[cnt].alloc_size = dsin[cnt].size * 3 +MAXDIGITS;
        if ((dsin[cnt].type & CREST_TYPE_INT32) && (dsin[cnt].type & CREST_TYPE_VECTOR))
        {
          dsin[cnt].alloc_size = dsin[cnt].size * sizeof(int32_t) * 3 +MAXDIGITS;
          p->cus[p->ncus].outds[cnt_out].size=dsin[cnt].size* sizeof(int32_t);

        }
        else if ((dsin[cnt].type & CREST_TYPE_DOUBLE) && (dsin[cnt].type & CREST_TYPE_VECTOR))
        {
          dsin[cnt].alloc_size = dsin[cnt].size * sizeof(double) * 3 + MAXDIGITS;
          p->cus[p->ncus].outds[cnt_out].size=dsin[cnt].size* sizeof(double);

        }
        else if ((dsin[cnt].type & CREST_TYPE_INT64) && (dsin[cnt].type & CREST_TYPE_VECTOR))
        {
          dsin[cnt].alloc_size = dsin[cnt].size * sizeof(int64_t) * 3 +MAXDIGITS;
          p->cus[p->ncus].outds[cnt_out].size=dsin[cnt].size* sizeof(int64_t);

        }

      }
        p->cus[p->ncus].outds[cnt_out].alloc_size=dsin[cnt].alloc_size;
        DPRINT("\"%s\" OUT allocating %d bytes size: %d 0x%x\n",p->cus[p->ncus].outds[cnt_out].name, p->cus[p->ncus].outds[cnt_out].alloc_size,dsin[cnt].size,dsin[cnt].type);
       // p->cus[p->ncus].maxbufsize+= p->cus[p->ncus].outds[cnt_out].alloc_size;

        p->cus[p->ncus].outds[cnt_out].data = calloc(1, dsin[cnt].alloc_size);
        if ((p->cus[p->ncus].outds[cnt_out].data == NULL) || (p->cus[p->ncus].outds[cnt_out].format == NULL))
        {
          printf("## cannot allocate memory for attribute data\n");
          return -6;
        }
        cnt_out++;
      }
    }

    p->ncus++;
    return p->ncus; // return the UID
  }

  static int update_attribute(cu_t * cu, int attr, void *data)
  {
    ds_t *p = cu->outds + attr;
    if ((attr >= cu->nout) || (attr < 0))
    {
      printf("## invalid attribute index for output, max is:%d\n", cu->nout - 1);
      return -7;
    }
    if (p->type == CREST_TYPE_INT32)
    {
      snprintf(p->data, p->alloc_size, p->format, *(int32_t *)data);
    }
    else if (p->type == CREST_TYPE_INT64)
    {
      snprintf(p->data, p->alloc_size, p->format, *(int64_t *)data);
    }
    else if (p->type == CREST_TYPE_DOUBLE)
    {
      snprintf(p->data, p->alloc_size, p->format, *(double *)data);
    }
    else if (p->type == CREST_TYPE_STRING)
    {
      snprintf(p->data, p->alloc_size, p->format, (char *)data);
    }
    else if ((p->type == CREST_TYPE_BINARY) || (p->type & CREST_TYPE_VECTOR))
    {
      char *bsa = b64_encode((char *)data, p->size);
      if (bsa)
      {
        int diff=strlen(bsa)-p->alloc_size;
        if(diff>0){
          diff+=strlen(p->format);
          p->alloc_size+=diff;
          p->data=realloc(p->data,p->alloc_size);
          
        }
        snprintf(p->data, p->alloc_size, p->format, bsa);
        free(bsa);
      }
    }
    
    DPRINT("updating [%d]\"%s\" format \"%s\" size:%d allocated:%d type:0x%x (@0x%x)\n", attr, p->data, p->format, p->size, p->alloc_size,p->type, data);
    return 0;
  }

  int chaos_crest_update(chaos_crest_handle_t h, uint32_t cu_uid, int attr_pos, void *data)
  {
    _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
    cu_t *cu;
    if ((cu_uid > p->ncus) || (cu_uid <= 0))
    {
      printf("## bad Id %d", cu_uid);
      return -8;
    }
    cu = p->cus + (cu_uid - 1);
    return update_attribute(cu, attr_pos, data);
  }

  int chaos_crest_update_by_name(chaos_crest_handle_t h, uint32_t cu_uid, char *attr_name, void *data)
  {
    _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
    cu_t *cu;
    int cnt;
    if ((cu_uid > p->ncus) || (cu_uid <= 0))
    {
      printf("## bad Id %d", cu_uid);
      return -8;
    }
    cu = p->cus + (cu_uid - 1);

    for (cnt = 0; cnt < cu->nout; cnt++)
    {
      if (!strcmp(cu->outds[cnt].name, attr_name))
      {
        return update_attribute(cu, cnt, data);
      }
    }
    return -9; // attribute not found
  }

  int chaos_crest_close(chaos_crest_handle_t h)
  {
    int cnt;
    _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
#ifndef MOONGOOSE
    http_client_deinit(p->http);
    close(p->sock_fd);
#else
    mg_mgr_free(&p->mgr);

#endif
    cu_t *cus = p->cus;

    for (cnt = 0; cnt < p->ncus; cnt++)
    {
      int cntt;
      if (cus[cnt].name)
      {
        free((void *)cus[cnt].name);
        cus[cnt].name = 0;
      }

      for (cntt = 0; cntt < cus[cnt].nout; cntt++)
      {
        if (cus[cnt].outds[cntt].name)
        {
          free((void *)cus[cnt].outds[cntt].name);
          cus[cnt].outds[cntt].name = 0;
        }
        if (cus[cnt].outds[cntt].desc)
        {
          free((void *)cus[cnt].outds[cntt].desc);
          cus[cnt].outds[cntt].desc = 0;
        }
        if (cus[cnt].outds[cntt].format)
        {
          free((void *)cus[cnt].outds[cntt].format);
          cus[cnt].outds[cntt].format = 0;
        }

        if (cus[cnt].outds[cntt].data)
        {
          free((void *)cus[cnt].outds[cntt].data);
          cus[cnt].outds[cntt].data = 0;
        }
      }

      for (cntt = 0; cntt < cus[cnt].nin; cntt++)
      {
        if (cus[cnt].inds[cntt].name)
        {
          free((void *)cus[cnt].inds[cntt].name);
          cus[cnt].inds[cntt].name = 0;
        }
        if (cus[cnt].inds[cntt].desc)
        {
          free((void *)cus[cnt].inds[cntt].desc);
          cus[cnt].inds[cntt].desc = 0;
        }
      }
      free((void *)cus[cnt].inds);
      free((void *)cus[cnt].outds);
      cus[cnt].inds = 0;
      cus[cnt].outds = 0;
    }
    free(p->cus);
    p->cus = 0;
    free(p->hostname);
    free(p->wan_url);
    free(h);
    return 0;
  }

  static int dump_attribute_desc(ds_t * attr, char *dir, char *buffer, int size, int last)
  {

    return snprintf(buffer, size, "{\"cudk_ds_attr_name\":\"%s\",\"cudk_ds_attr_desc\":\"%s\",\"cudk_ds_attr_type\":\"%s\",\"cudk_ds_attr_dir\":\"%s\"}%s",
                    attr->name, attr->desc, typeToString(attr->type), dir, last ? "" : ",");
  }

  static int dump_attribute_value(ds_t * attr, char *buffer, int size, int last)
  {

    return snprintf(buffer, size, "%s%s",
                    attr->data, last ? "" : ",");
  }
  /*
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
    p->npush=0;
    DPRINT("registering CU \"%s\" UID:%d IN:%d, OUT:%d\n",cu->name,cu_uid,cu->nin,cu->nout);
    ts=getEpoch();
    snprintf(buffer,size,"{\"ndk_uid\":\"%s\",\"cudk_ds_timestamp\":%llu,\"cudk_ds_desc\":[",cu->name,ts);
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
    //    snprintf(url,sizeof(url),"/api/v1/producer/register/%s",cu->name);
    snprintf(url,sizeof(url),"/api/v1/producer/register");
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
*/

  static int push_cu_int(chaos_crest_handle_t h, uint32_t cu_uid, int mode,int datasetType)
  {
    _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
    cu_t *cu;
    int cnt,size=0;
    char *pnt;
    char url[256];
    int err;
    int csize;
    uint64_t ts;
    char*buffer;
    int nports=0;
    ds_t *dataset=0;

    if ((cu_uid > p->ncus) || (cu_uid <= 0))
    {
      printf("## bad Id %d", cu_uid);
      return -8;
    }

    cu = p->cus + (cu_uid - 1);
    ts = getEpoch();
    nports=(datasetType==0)?cu->nout:cu->nin;
    dataset=(datasetType==0)?cu->outds:cu->inds;
    for (cnt = 0; cnt < nports; cnt++){
        ds_t * attr=dataset + cnt;
        size+= attr->alloc_size;
    }
    
    
    buffer=(char*)malloc(size);
    if (mode > 0)
    {
      snprintf(url, sizeof(url), "/api/v1/producer/jsoninsert");
      snprintf(buffer, size, "{\"ndk_uid\":\"%s\",\"dpck_ds_type\":%d,\"dpck_seq_id\":{\"$numberLong\":\"%llu\"},\"dpck_ats\":{\"$numberLong\":\"%llu\"}%s", cu->name, datasetType, p->npush, ts, nports > 0 ? "," : "");

    }
    else
    {
      p->npush=0;
      snprintf(url, sizeof(url), "/api/v1/producer/jsonregister");
      snprintf(buffer, size, "{\"ndk_uid\":\"%s\",\"dpck_ds_type\":%d,\"cudk_ds_attr_dir\":%d,\"dpck_seq_id\":{\"$numberLong\":\"%llu\"},\"dpck_ats\":{\"$numberLong\":\"%llu\"}%s", cu->name, datasetType,(datasetType==0)?1:0, p->npush, ts, nports > 0 ? "," : "");

    }
  //  snprintf(buffer, size, "{\"ndk_uid\":\"%s\",\"dpck_ds_type\":%d,\"dpck_seq_id\":{\"$numberLong\":\"%llu\"}%s", cu->name, 0, p->npush, cu->nout > 0 ? "," : "");

    for (cnt = 0; cnt < nports; cnt++)
    {
      csize = strlen(buffer);
      pnt = buffer + csize;
      dump_attribute_value(dataset + cnt, pnt, size - csize, ((cnt + 1) == nports));
    }
    strcat(buffer, "}");

    //    snprintf(url,sizeof(url),"/api/v1/producer/insert/%s",cu->name);
    
    char buffer_rx[8192];
    if ((err = http_post(h, url, buffer, strlen(buffer), buffer_rx, sizeof(buffer_rx))) == 0)
    {
      uint32_t t = (getEpoch() - ts);
      p->tot_push_time += t;
      p->max_push = (t > p->max_push) ? t : p->max_push;
      p->min_push = (t < p->min_push) && (t > 0) ? t : p->min_push;
      p->npush++;
      free(buffer);
      return 0;
    }
    else
    {
      printf("## post ret: %d to:\"%s\" of:\"%s\" result:\"%s\"", err, url, buffer, buffer_rx);
    }
    free(buffer);

    return -9; // registration failure
  }
  static int register_cu(chaos_crest_handle_t h, uint32_t cu_uid, char *buffer, int size)
  {
    _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;

    cu_t *cu;
    int cnt;
    char *pnt;
    char url[256];
    int err;
    int csize;
    unsigned long long ts;
    int ret;
    if ((cu_uid > p->ncus) || (cu_uid <= 0))
    {
      printf("## bad Id %d", cu_uid);
      return -8;
    }

    cu = p->cus + (cu_uid - 1);
    for (cnt = 0; cnt < cu->nout; cnt++)
    {
      ds_t *p = cu->outds + cnt;
      //      printf("format : %d->%s\n",p->type,p->format);

      if (p->type == CREST_TYPE_INT32)
      {
        snprintf(p->data, p->alloc_size, p->format, 0);
      }
      else if (p->type == CREST_TYPE_INT64)
      {
        snprintf(p->data, p->alloc_size, p->format, 0);
      }
      else if (p->type == CREST_TYPE_DOUBLE)
      {
        snprintf(p->data, p->alloc_size, p->format, 1.2);
      }
      else {
        snprintf(p->data, p->alloc_size, p->format, "");
      }
    }
      
      ret=push_cu_int(h, cu_uid, 0,0);

     for (cnt = 0; cnt < cu->nin; cnt++)
    {
      ds_t *p = cu->inds + cnt;
      //      printf("format : %d->%s\n",p->type,p->format);

      if (p->type == CREST_TYPE_INT32)
      {
        snprintf(p->data, p->alloc_size, p->format, 0);
      }
      else if (p->type == CREST_TYPE_INT64)
      {
        snprintf(p->data, p->alloc_size, p->format, 0);
      }
      else if (p->type == CREST_TYPE_DOUBLE)
      {
        snprintf(p->data, p->alloc_size, p->format, 1.2);
      }
      else {
        snprintf(p->data, p->alloc_size, p->format, "");
      }
    }
    if(cu->nin){
      ret+= push_cu_int(h, cu_uid, 0,1);
    }
    return ret;
  }

    static int push_cu(chaos_crest_handle_t h, uint32_t cu_uid,int dstype)
    {
      return push_cu_int(h, cu_uid, 1,dstype);
    }
    int chaos_crest_register(chaos_crest_handle_t h, uint32_t cu_cuid)
    {
      _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
      int ret;
      char buffer[MAXBUFFER];
      if (cu_cuid == 0)
      {
        int cnt;
        for (cnt = 0; cnt < p->ncus; cnt++)
        {
          if ((ret = register_cu(h, cnt + 1, buffer, sizeof(buffer))) != 0)
          {
            return ret;
          }
        }
        return 0;
      }

      return register_cu(h, cu_cuid, buffer, sizeof(buffer));
    }

    int chaos_crest_push(chaos_crest_handle_t h, uint32_t cu_uid,int dstype)
    {
      _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
      int ret;
     // char buffer[MAXBUFFER];
      if (cu_uid == 0)
      {
        int cnt;
        for (cnt = 0; cnt < p->ncus; cnt++)
        {
          if ((ret = push_cu(h, cnt + 1,dstype)) != 0)
          {
            return ret;
          }
        }
        return 0;
      }

      return push_cu(h, cu_uid,dstype);
    }

    float chaos_crest_push_time(chaos_crest_handle_t h, uint32_t * max, uint32_t * min)
    {
      _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
      if (max)
      {
        *max = p->max_push;
      }
      if (min)
      {
        *min = p->min_push;
      }

      if (p->npush > 0)
      {
        return 1.0 * (float)p->tot_push_time / p->npush;
      }
      return 0;
    }

    float chaos_crest_reg_time(chaos_crest_handle_t h)
    {
      _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
      if (p->nreg > 0)
      {
        return 1.0 * (float)p->tot_registration_time / p->nreg;
      }
      return 0;
    }

    int chaos_crest_cu_cmd(chaos_crest_handle_t h, const char *cuname, const char *cmd, const char *args)
    {
      char command[1024];
      int ret;
      _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
      if (cmd == 0 || cuname == 0)
        return -1;

      if (args)
      {
        sprintf(command, "/CU?dev=%s&cmd=%s&parm=%s", cuname, cmd, args);
      }
      else
      {
        sprintf(command, "/CU?dev=%s&cmd=%s", cuname, cmd);
      }
#ifndef MOONGOOSE

      ret = http_request(p->http, "GET", p->wan_url, "chaos_crest_cu_cmd", command, "application/text", "", 0, 0);
#endif
      if (ret == 200)
        return 0;

      return ret;
    }

    static char *getBsonValue_r(char *input, char *key, char *buffer, int maxsize)
    {
      int cnt = 0;
      char match[256];
      char *pnt;
      snprintf(match, sizeof(match), "\"%s\" :", key);
      pnt = strstr(input, match);
      if (pnt == 0)
      {
        snprintf(match, sizeof(match), "%s:", key);
        pnt = strstr(input, match);
      }

      if (pnt)
      {
        int brace = 0;

        pnt += strlen(match);
        while ((*pnt != 0) && (*pnt != ',') && cnt < maxsize)
        {
          if (*pnt == ' ' || *pnt == '\"')
          {
            pnt++;
            continue;
          }

          if (*pnt == '{')
          {
            brace++;
            if (brace == 1)
            {
              //remove the first open brace
              pnt++;
              continue;
            }
          }

          if (*pnt == '}')
          {
            if (brace == 1)
            {
              //last close brace end
              buffer[cnt] = 0;
              break;
            }
            brace--;
          }

          buffer[cnt++] = *pnt;
          pnt++;
        }
      }
      buffer[cnt] = 0;
      return buffer;
    }
    //remove spaces and "
    static void clean_bson(char *buf)
    {
      char *dst = buf;
      char *src = buf;
      while (*src != 0)
      {
        if (*src == '\"')
        {
          src++;
          continue;
        }
        *dst++ = *src++;
      }
      *dst = 0;
    }

    static char *getBsonValue(char *input, char *key)
    {
      static char buffer[1024];
      return getBsonValue_r(input, key, buffer, sizeof(buffer));
    }
    uint64_t chaos_crest_cu_get(chaos_crest_handle_t h, const char *cuname, char *output, int maxsize)
    {

      char cmd[256];
      _chaos_crest_handle_t *p = (_chaos_crest_handle_t *)h;
      int ret;
      if (output == 0 || cuname == 0)
        return 0;
      sprintf(cmd, "/CU?dev=%s&cmd=status", cuname);
#ifndef MOONGOOSE

      ret = http_request(p->http, "GET", p->wan_url, "chaos_crest_cu_get", cmd, "html/text", 0, output, maxsize);
#endif
      if (ret < 0)
      {
        printf("## error getting cu %s\n", cuname);
      }

      if (ret == 200)
      {
        char *tmp = getBsonValue(output, "dpck_ts");

        tmp = getBsonValue(tmp, "$numberLong");
        if (*tmp != 0)
          return atoll(tmp);
      }

      return 0;
    }
    uint64_t chaos_crest_cu_get_key_value(chaos_crest_handle_t h, const char *cuname, char *keys, char *values, int maxsize)
    {
      uint64_t rett = 0;

      char tmpbuffer[maxsize * 2];

      if (keys == 0 || cuname == 0 || values == 0)
        return 0;
      rett = chaos_crest_cu_get(h, cuname, tmpbuffer, sizeof(tmpbuffer));

      if (rett > 0)
      {
        clean_bson(tmpbuffer);
        char *pnt = strtok(tmpbuffer, ",");
        *keys = 0;
        *values = 0;
        while (pnt != 0)
        {
          char key[256];
          char val[256];
          char type[256];
          int found = 0;

          if (sscanf(pnt, "%s : { %s : %s }", key, type, val) == 3)
          {
            found = 1;
          }
          else if (sscanf(pnt, "%s : %s", key, val) == 2)
          {
            found = 1;
          }

          if (found)
          {
            if (*keys == 0)
            {
              sprintf(keys, "%s", key);
            }
            else
            {
              sprintf(keys, "%s,%s", keys, key);
            }
            if (*values == 0)
            {
              sprintf(values, "%s", val);
            }
            else
            {
              sprintf(values, "%s,%s", values, val);
            }
          }
          pnt = strtok(NULL, ",");
        }
        return rett;
      }

      return 0;
    }

    uint64_t chaos_crest_cu_get_channel(chaos_crest_handle_t h, const char *cuname, const char *channame, char *output, int maxsize)
    {
      char buf[MAXBUFFER];
      uint64_t ret;
      ret = chaos_crest_cu_get(h, cuname, buf, sizeof(buf));
      if (ret > 0)
      {

        char *pnt = getBsonValue_r(buf, (char *)channame, output, maxsize);
        if (*pnt != 0)
        {
          return ret;
        }
      }

      return 0;
    }
