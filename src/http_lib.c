/** http_lib.c
 * @author Andrea Michelotti
 * @version 0.1
 * @date 26/2/2015
 * Simple apis to make http post
 */
#include <time.h>
#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifdef HTTP_LIB_DEBUG
#define DPRINT(x,ARGS...) printf(x, ##ARGS)
#else
#define DPRINT(x,...)
#endif

#include "http_lib.h"
static volatile unsigned long *brecv = 0, *bsent = 0;

int http_init_stats(unsigned long* byte_sent, unsigned long* byte_recv) {
    brecv = byte_recv;
    bsent = byte_sent;
    return 0;
}

#define SEND_STR(sock,buffer,STR,ARGS...) {\
  int ret;\
  if(STR!=NULL){\
    snprintf(buffer,sizeof(buffer),STR, ## ARGS);	}		\			
if((ret=write(sock,buffer,strlen(buffer)))!=strlen(buffer)) {if((ret>0) && bsent) *bsent+=ret;DPRINT("## Error sending ret= %d, exp %d\n",ret,strlen(buffer));return -4;} \
  DPRINT("->(%d)%s\n",ret,buffer);				\
      if(bsent) (*bsent)+=ret;}

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

static int getResponseHeader(int sock, int retryn, int*size_body,int*encoding) {
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
        if (brecv) (*brecv) += ret;
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
		DPRINT("Header Ends %d bytes received\n",cntb);
		return retcode;
	      }
            }
            if (term) {
                cnt = 0;
		//                DPRINT("-->%s",stringa);
                if (sscanf(stringa, "HTTP/1.1 %d", &retcode) == 1) {
                    DPRINT("got ret code %d\n", retcode);
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

int getResponse(int sock, char*buffer, int max_size) {
    int ret;
    int retcode = HTTP_ERROR_PARSING_POST_RESPONSE, response_size = 0;
    int cnt = 0, cntb = 0,copy_buffer=0,enc_cnt=0;
    int retry = HTTP_POST_FILE_RETRY;
    char buf;
    char encoding_buf[256];
    char debug_buffer[4096];
    int encoding;
    char last_char=0;
    int term;
    int fetch_term=0;
    int fetch_blank_line=0;
    *buffer = 0;
    if ((retcode = getResponseHeader(sock, HTTP_POST_FILE_RETRY, &response_size,&encoding)) > 0) {
      if(response_size==0){
	DPRINT("Length unspecified\n");
      }
      if(encoding==1){
	copy_buffer=0;
      } else {
	copy_buffer = response_size;
      }
      
        while (((ret = read(sock, &buf, 1)) >= 0) && (retry > 0)) {
            if (brecv) (*brecv) += ret;
            if (ret == 0) {
                retry--;
                DPRINT("Timeout reading retry %d", retry);
                usleep(200000);
                continue;
            } else if (ret > 0) {
	      term=((buf=='\n') && (last_char=='\r'));
	      last_char=buf;
	      if(term){
		debug_buffer[cntb-1]=0;
		DPRINT("debug(%d)=>\"%s\"\n",strlen(debug_buffer),debug_buffer);
		cntb=0;

	      } else{
		debug_buffer[cntb++]=buf;
	      }
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
		fetch_term=1;
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


int http_request(int sock, const char*method,char* hostname, const char*agent,char* api, char*content,char* parameters, char* message, int size) {
    char buffer[4096];
    *buffer = 0;
    DPRINT("http_request (%d) method:%s hostname:%s api:%s, max_size %d\n",sock,method,hostname,api,size);
    SEND_STR(sock, buffer, "%s %s HTTP/1.1\r\n", method,api);
    SEND_STR(sock, buffer, "%s", "Accept: */*\r\n");
    SEND_STR(sock, buffer, "%s", "User-Agent: %s\r\n",agent);
    SEND_STR(sock, buffer, "%s", "Connection: Keep-Alive\r\n");
    SEND_STR(sock, buffer, "Content-Length: %d\r\n", (int) strlen(parameters));
    SEND_STR(sock, buffer, "%s", "Accept-Language: en-us\r\n");
    SEND_STR(sock, buffer, "%s", "Accept-Encoding: gzip, deflate\r\n");
    SEND_STR(sock, buffer, "Host:%s\r\n", hostname);
    SEND_STR(sock, buffer, "Content-Type: %s\r\n",content);


    SEND_STR(sock, buffer, "\r\n%s\r\n", parameters);

    //    fflush(fdopen(sock,"w+"));
    //    shutdown(sock,SHUT_WR);
    return getResponse(sock, message, size);
}
