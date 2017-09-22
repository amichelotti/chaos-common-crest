/** http_lib.h
 * @author Andrea Michelotti
 * @version 0.1
 * @date 26/2/2015
 * Simple apis to make http post
 */
#ifndef _HTTP_POST_H_
#define _HTTP_POST_H_
typedef void* http_handle_t;
/** initialize the http handle 
 
 */
http_handle_t http_client_init(int sock);
void http_client_deinit(http_handle_t h);
int http_perform_request(http_handle_t h,const char*method,const char* hostname, const char*agent,const char* api, const char*content, const char* parameters);
int http_get_answer(http_handle_t h, char* message, int size);
int http_wait_answer(http_handle_t h, char* message, int size);

int http_request(http_handle_t h, const char*method,const char* hostname, const char*agent,const char* api, const char*content, const char* parameters, char* message, int size);

#define HTTP_GET_FILE_RETRY 50
#define HTTP_POST_FILE_RETRY 30

#define HTTP_BUF_SIZE 256
#define HTTP_ERROR_TIMEOUT -100
#define HTTP_ERROR_CREATE_FILE -10
#define HTTP_ERROR_MD5_CALC -14
#define HTTP_ERROR_MD5 -12
#define HTTP_ERROR_READING -11
#define HTTP_ERROR_CANNOT_STAT_FILE -13
#define HTTP_ERROR_CANNOT_OPEN_FILE -3
#define HTTP_ERROR_READING_HEADER -15
#define HTTP_ERROR_PARSING_HEADER -111
#define HTTP_ERROR_PARSING_POST_RESPONSE -5
#define HTTP_ERROR_CONNECTION_DROP -6




#endif
