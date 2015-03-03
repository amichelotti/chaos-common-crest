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
#define DPRINT(x,...) printf(x, ARGS...)
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
    DPRINT("->(%d)%s\n",strlen(buffer),buffer);			\
    if((ret=write(sock,buffer,strlen(buffer)))!=strlen(buffer)) {if((ret>0) && bsent) *bsent+=ret;DPRINT("## Error sending ret= %d, exp %d\n",ret,strlen(buffer));return -4;} \
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

static int getResponseHeader(int sock, int retryn, int*size_body) {
    int ret, cnt=0, retcode = HTTP_ERROR_PARSING_HEADER;
    int response_size;
    int retry = retryn;
    int cntb=0;
    char stringa[256];
    char buf;
    while (((ret = read(sock, &buf, 1)) >= 0) && (retry > 0)) {
        if (brecv) (*brecv) += ret;
        if (ret <= 0) {
            retry--;
            DPRINT("Timeout reading retry %d", retry);
            usleep(200000);
            continue;
        } else if (ret > 0) {
            retry = HTTP_GET_FILE_RETRY;
            cntb++;
            if (cnt<sizeof (stringa)) {
                if (buf == '\n') {
                    stringa[cnt] = 0;
                    DPRINT("%s\n", stringa);
                } else
                    stringa[cnt++] = buf;
            }
            if (buf == '\n') {
                cnt = 0;
                DPRINT("-->%s",stringa);
                if (sscanf(stringa, "HTTP/1.1 %d", &retcode) == 1) {
                    DPRINT("got ret code %d", retcode);
                } else if (sscanf(stringa, "Content-Length:%d", &response_size) == 1) {
                    //  printf("get: got length %d\n",response_size);
                    DPRINT("got length %d", response_size);
                    *size_body = response_size;
                } else if (strstr(stringa, "NO CARRIER")) {
                    DPRINT("connection dropped");
                    return HTTP_ERROR_CONNECTION_DROP;
                } else if (*stringa == '\r' /*&& (retcode!=HTTP_ERROR_PARSING_HEADER)*/) {
                    DPRINT("Header Ends %d bytes received",cntb);
                    return retcode;
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
    int cnt = 0, cntb = 0;
    int retry = HTTP_POST_FILE_RETRY;
    char buf;
    *buffer = 0;
    if ((retcode = getResponseHeader(sock, HTTP_POST_FILE_RETRY, &response_size)) > 0) {
        while (((ret = read(sock, &buf, 1)) >= 0) && (retry > 0)) {
            if (brecv) (*brecv) += ret;
            if (ret == 0) {
                retry--;
                DPRINT("Timeout reading retry %d", retry);
                usleep(200000);
                continue;
            } else if (ret > 0) {
                retry = HTTP_POST_FILE_RETRY;
                if (cnt < max_size-1) {
                    buffer[cnt++] = buf;
                    buffer[cnt]=0;
                 
                }
                if(buf=='\n'){
                    DPRINT("%d] --> %s",buffer);
                }
                cntb++;
                if (cntb == response_size){
                    DPRINT("done received %d",cntb);
                    break;
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


int http_request(int sock, char* hostname, char* api, char*content,char* parameters, char* message, int size) {
    char buffer[1024];
    *buffer = 0;

    SEND_STR(sock, buffer, "POST %s HTTP/1.1\r\n", api);
    SEND_STR(sock, buffer, "%s", "Accept: */*\r\n");
    SEND_STR(sock, buffer, "%s", "User-Agent: Mozilla/4.0\r\n");
    SEND_STR(sock, buffer, "%s", "Connection: Keep-Alive\r\n");
    SEND_STR(sock, buffer, "Content-Length: %d\r\n", (int) strlen(parameters));
    SEND_STR(sock, buffer, "%s", "Accept-Language: en-us\r\n");
    SEND_STR(sock, buffer, "%s", "Accept-Encoding: gzip, deflate\r\n");
    SEND_STR(sock, buffer, "Host:%s\r\n", hostname);
    SEND_STR(sock, buffer, "%s\r\n", "Content-Type: %s",content);


    SEND_STR(sock, buffer, "\r\n%s\r\n", parameters);


    return getResponse(sock, message, size);
}
