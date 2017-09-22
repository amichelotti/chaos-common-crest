/** http_lib.c
 * @author Andrea Michelotti
 * @version 0.1
 * @date 26/2/2015
 * Simple apis to make http post
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <assert.h>
#define MAXBUFFER 8192

typedef struct _http {
   int sock;
   pthread_t pid;
   pthread_mutex_t mutex;
   pthread_cond_t cond;
   char answer[MAXBUFFER];
   int retcode;
   int exit;
   unsigned long sendid;
   unsigned long recvid;
   unsigned long sentb;
   unsigned long recvb;
} _http_handle_t;


#ifdef HTTP_LIB_DEBUG
#define DPRINT(x,ARGS...) printf(x, ##ARGS)
#else
#define DPRINT(x,...)
#endif

#include "http_lib.h"
static int getResponse(http_handle_t h, char*buffer, int max_size);

static void* receive(void*pp){
    _http_handle_t* p=(_http_handle_t*)pp;
    while(p->exit==0){
        p->retcode=getResponse(pp,p->answer,MAXBUFFER);
	DPRINT("received %d %d, retcode %d,%s\n",p->sendid,p->recvid,p->retcode,p->answer);
        pthread_cond_signal(&p->cond);
    }
    return NULL;
}
http_handle_t http_client_init(int sock){
    _http_handle_t* p=calloc(1,sizeof(_http_handle_t));
    assert(p);
    assert(sock);
    p->sock=sock;
    pthread_mutex_init(&p->mutex, NULL);
    pthread_cond_init(&p->cond,NULL);
    if(pthread_create(&p->pid,NULL,receive,(void*)p)==0){
        return p;
    }
    free((void*)p);
    return 0;
}


void http_client_deinit(http_handle_t h){
    _http_handle_t* p=h;
    p->exit=1;
    pthread_cond_broadcast(&p->cond);
    pthread_cond_destroy(&p->cond);
    pthread_mutex_destroy(&p->mutex);
    pthread_cancel(p->pid);
    //    pthread_join(p->pid,NULL);
    free(p);
}

int http_get_answer(http_handle_t h, char* message, int size){
    _http_handle_t* p=(_http_handle_t*)h;
    pthread_mutex_lock(&p->mutex);
    DPRINT("get answer (%d) retcode:%d\n",strlen(p->answer),p->retcode);
    memcpy(message,p->answer,size<strlen(p->answer)?size:strlen(p->answer) +1);
    pthread_mutex_unlock(&p->mutex);
    return p->retcode;
}
int http_wait_answer(http_handle_t h, char* message, int size){
    _http_handle_t* p=(_http_handle_t*)h;
    pthread_mutex_lock(&p->mutex);
    DPRINT("sent %d recv %d\n",p->sendid,p->recvid);
    if(p->sendid>=p->recvid){
      DPRINT("waiting answer\n");
      pthread_cond_wait(&p->cond,&p->mutex);
    }
    pthread_mutex_unlock(&p->mutex);
    return http_get_answer(h,message,size);
}



#define SEND_STR(sock,buffer,STR,ARGS...) {\
  int ret;\
  if(STR!=NULL){\
    snprintf(buffer,sizeof(buffer),STR, ## ARGS);	}		\
if((ret=write(sock,buffer,strlen(buffer)))!=strlen(buffer)) {if((ret>0) && bsent) *bsent+=ret;DPRINT("## Error sending ret= %d, exp %d\n",ret,strlen(buffer));return -4;} \
  DPRINT("->(%d)%s\n",ret,buffer);				\
      if(bsent) (*bsent)+=ret;}

#define ADD_HEADER_STR(_buf,maxsize,STR,ARGS...) {	\
    _buf+=snprintf(_buf,maxsize-strlen(_buf),STR,## ARGS);}

static int add_form_data(char*buffer, char*name, char*val, char*delim) {
    char temp[1024];
    *temp = 0;
    snprintf(temp, sizeof (temp), "--%s\r\n", delim);
    sprintf(temp, "%scontent-disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", temp, name, val);
    strcat(buffer, temp);
    //  printf("(%d)->%s\n",strlen(temp),temp);
    return strlen(temp);
}

static int add_form_file_data(char*buffer, char*name, char*fname, char*delim) {
    char temp[1024];
    *temp = 0;
    snprintf(temp, sizeof (temp), "--%s\r\n", delim);
    sprintf(temp, "%scontent-disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: text/plain\r\n\r\n", temp, name, fname);
    strcat(buffer, temp);
    //  printf("(%d)->%s\n",strlen(temp),temp);
    return strlen(temp);
}

static int getResponseHeader(http_handle_t h, int retryn, int*size_body,int*encoding) {
  _http_handle_t* p=(_http_handle_t*)h;
  int sock=p->sock;
  int ret, cnt=0, retcode = HTTP_ERROR_PARSING_HEADER;
  int response_size;
  int retry = retryn;
  int cntb=0;
    char stringa[256];
    char sencoding[256];
    char buf;
    int term;
    char last_char;
    *size_body=0;
    *encoding=0;
    while (((ret = read(sock, &buf, 1)) >= 0) && (retry > 0)) {
	p->recvb+=ret;
        if (ret <= 0) {
            retry--;
            DPRINT("Timeout reading retry %d\n", retry);
            usleep(200000);
            continue;
        } else if (ret > 0) {
            retry = HTTP_GET_FILE_RETRY;
            cntb++;
	    term=((buf=='\n') && (last_char=='\r'));
	    last_char=buf;
            if (cnt<sizeof (stringa)) {
	      if (term) {
		stringa[cnt-1] = 0;
		DPRINT("header=>\"%s\"\n", stringa);
	      } else{
		stringa[cnt++] = buf;
	      }
	      if (*stringa== 0) {
		DPRINT("Header Ends %d bytes received, retcode %d\n",cntb,retcode);
		return retcode;
	      }
            }
            if (term) {
              int minor;  
	      cnt = 0;
		
		//                DPRINT("-->%s",stringa);
	      if (sscanf(stringa, "HTTP/1.%d %d", &minor,&retcode) == 2) {
		DPRINT("got ret code %d, minor %d\n", retcode,minor);
                } else if (sscanf(stringa, "Content-Length:%d", &response_size) == 1) {
                    //  printf("get: got length %d\n",response_size);
                    DPRINT("got length %d\n", response_size);
                    *size_body = response_size;
                } else if (sscanf(stringa,"Transfer-Encoding:%s",sencoding)){
		  if(strstr(sencoding,"chunked")){
		    *encoding=1;
		    DPRINT("chunked encoding\n");
		  }
		} else if (strstr(stringa, "NO CARRIER")) {
                    DPRINT("connection dropped");
                    return HTTP_ERROR_CONNECTION_DROP;
                }
            }
        } else {
            DPRINT("error reading");
            
            return HTTP_ERROR_READING_HEADER;
        }
    }
    if (retry < 0) {
        DPRINT("Timeout ");
        return HTTP_ERROR_TIMEOUT;
    }
    return retcode;
}

int getResponse(http_handle_t h, char*buffer, int max_size) {
  _http_handle_t* p=(_http_handle_t*)h;
  int sock=p->sock;
  int ret;
  int retcode = HTTP_ERROR_PARSING_POST_RESPONSE, response_size = 0;
  int cnt = 0, copy_buffer=0,enc_cnt=0;
  int retry = HTTP_POST_FILE_RETRY;
    char buf;
    char encoding_buf[256];
#ifdef HTTP_LIB_DEBUG
    char debug_buffer[4096];
#endif
    int encoding;
    char last_char=0;
    int term;
    int fetch_term=0;
    int fetch_blank_line=0;
    
    if ((retcode = getResponseHeader(h, HTTP_POST_FILE_RETRY, &response_size,&encoding)) > 0) {
      if(response_size==0){
	DPRINT("Length unspecified\n");
      } else{
	DPRINT("Length :%d\n",response_size);
      }
      if(encoding==1){
	copy_buffer=0;
      } else {
	copy_buffer = response_size;
      }
      pthread_mutex_lock(&p->mutex);
      *buffer = 0;
      p->recvid++;
      while (((ret = read(sock, &buf, 1)) >= 0) && (retry > 0)) {
	p->recvb+=ret;
	if (ret == 0) {
	  retry--;
	  DPRINT("Timeout reading retry %d", retry);
	  usleep(20000);
	  continue;
	} else if (ret > 0) {
	  term=((buf=='\n') && (last_char=='\r'));
	  last_char=buf;
#ifdef HTTP_LIB_DEBUG

	  if(term){
	    debug_buffer[cntb-1]=0;
	    DPRINT("debug(%d)=>\"%s\"\n",strlen(debug_buffer),debug_buffer);
	    cntb=0;
	    
	  } else{
	    debug_buffer[cntb++]=buf;
	  }
#endif
	  if(fetch_blank_line && term){
	    DPRINT("end of body\n");
	    break;
	  }
	  if(fetch_term && term){
	    DPRINT("fetching terminator after buffer (%d/%d/%d)\"%s\"\n",strlen(buffer),cnt,max_size,buffer);
	    fetch_term=0;
	    continue;
	  }
	  retry = HTTP_POST_FILE_RETRY;
	  if((copy_buffer>0)){
	    if((cnt < (max_size-1))){
	      buffer[cnt++] = buf;
	      buffer[cnt]=0;
	    }
	    copy_buffer--;

	    if(cnt == response_size){
	      DPRINT("done received %d.",cnt);
	      break;
	    } else{
	      fetch_term=1;
	    }
	  } else {
	    if(encoding==1){
	      if(term){
		if(enc_cnt>0){
		  encoding_buf[enc_cnt-1]=0;
		  copy_buffer=strtoul(encoding_buf,0,16);
		  if(copy_buffer == 0){
		    DPRINT("done after encoding received %d",cnt);
		    fetch_blank_line=1;
		  }
		  DPRINT("encode still %d bytes\n",copy_buffer);
		  enc_cnt=0;
		}
	      } else {
		if(enc_cnt<sizeof(encoding_buf)-1){
		  encoding_buf[enc_cnt++]=buf;
		}
	      }
	    } else {
	      DPRINT("done received %d",cnt);
	      fetch_blank_line=1;
	      
	    }
	  }	  
	} else {
	      //an error occurred
	  pthread_mutex_unlock(&p->mutex);
	  if(cnt==response_size){
	    DPRINT("%% error occurred but the transfer is ended %d bytes, ret=%d",cnt,retcode);
	    return retcode;
	  } else {
	    return -5;
	  }
	}
      }
      if (retry < 0) {
	DPRINT("timeout");
	return HTTP_ERROR_TIMEOUT;
      }
    }
    pthread_mutex_unlock(&p->mutex);
    return retcode;
}


/*
========================
POST /path/to/script.php HTTP/1.0
Host: example.com
Content-type: multipart/form-data, boundary=AaB03x
Content-Length: $requestlen

--AaB03x
content-disposition: form-data; name="field1"

$field1
--AaB03x
content-disposition: form-data; name="field2"

$field2
--AaB03x
content-disposition: form-data; name="userfile"; filename="$filename"
Content-Type: $mimetype
Content-Transfer-Encoding: binary

$binarydata
--AaB03x--
 */

int http_perform_request(http_handle_t h,const char*method,const char* hostname, const char*agent,const char* api, const char*content, const char* parameters){
  _http_handle_t* p=(_http_handle_t*)h;
  int ret; 
  char buffer[4096];
  char *pnt=buffer;
  *buffer = 0;
  DPRINT("http_request (%d) method:%s hostname:%s api:%s, max_size %d\n",p->sock,method,hostname,api,sizeof(buffer));
  ADD_HEADER_STR(pnt,sizeof(buffer), "%s %s HTTP/1.1\r\n", method,api);
  ADD_HEADER_STR(pnt,sizeof(buffer), "Accept: */*\r\n");
  ADD_HEADER_STR(pnt,sizeof(buffer), "User-Agent: %s\r\n",agent);
  ADD_HEADER_STR(pnt,sizeof(buffer), "Connection: Keep-Alive\r\n");
  ADD_HEADER_STR(pnt,sizeof(buffer), "Host:%s\r\n", hostname);

  if((parameters!=0) && (*parameters!=0)) {
    ADD_HEADER_STR(pnt,sizeof(buffer), "Accept-Language: en-us\r\n");
    ADD_HEADER_STR(pnt,sizeof(buffer), "Accept-Encoding: gzip, deflate\r\n");

    ADD_HEADER_STR(pnt,sizeof(buffer), "Content-Type: %s\r\n",content);
    ADD_HEADER_STR(pnt,sizeof(buffer), "Content-Length: %d\r\n", (int) strlen(parameters));
    ADD_HEADER_STR(pnt,sizeof(buffer), "\r\n%s\r\n", parameters);
  } else {
    ADD_HEADER_STR(pnt,sizeof(buffer), "\r\n");
  }
  DPRINT("buffer :%s\n",buffer);
  if((ret=write(p->sock,buffer,strlen(buffer)))!=strlen(buffer)){
    DPRINT("#error sending:\"%s\", ret =%d\n",buffer,ret);

    return -4;
    } else {
    DPRINT("sent header and body :\n%s\n",buffer);
    p->sentb+=ret;
    p->sendid++;
  }

  return ret;
}
 
 int http_request(http_handle_t h, const char*method,const char* hostname, const char*agent,const char* api, const char*content,const char* parameters, char* message, int size) {
   int ret;
   
   
   if((ret=http_perform_request(h,method,hostname, agent,api, content, parameters))>0){
     return http_wait_answer(h,message,size);
   }

   return ret;
 }


