/** http_lib.h
 * @author Andrea Michelotti
 * @version 0.1
 * @date 26/2/2015
 * Simple apis to make http post
 */
#ifndef _HTTP_POST_H_
#define _HTTP_POST_H_

int http_request(int sock, const char*method,char* hostname, const char*agent,char* api, char*content, char* parameters, char* message, int size);

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