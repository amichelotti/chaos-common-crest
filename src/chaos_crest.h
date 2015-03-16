/** chaos_crest.h
 * @author Andrea Michelotti
 * @version 0.1
 * @date 26/2/2015
 * Simple apis to access chaos using http rest
 */
#ifndef _CHAOS_CREST_H_
#define _CHAOS_CREST_H_

#ifdef LABVIEW
typedef unsigned long long uint64_t;
typedef unsigned long uint32_t;
typedef int int32_t;

#else
#include <stdint.h>
#endif 


typedef void* chaos_crest_handle_t;

typedef struct _chaos_ds{
    const char *name;
    const char *desc;
    int        dir;
    int        type;
    int        size;
} chaos_ds_t;

#define TYPE_INT32 0x1
#define TYPE_INT64 0x2
#define TYPE_DOUBLE 0x4
#define TYPE_STRING 0x8
#define TYPE_BUFFER 0x10
#define TYPE_VECTOR 0x1000

#define DIR_INPUT 0
#define DIR_OUTPUT 1
#define DIR_IO 2


#define DEFINE_CU_DATASET(_x) \
static chaos_ds_t dataset_ ## _x[]={

#define DEFINE_ATTRIBUTE(name,desc,dir,type,size) \
{name,desc,dir,type},

#define END_CU_DATASET }


#define ADD_CU(handle,name) \
chaos_crest_add_cu(handle,#name,dataset_ ##name,sizeof(dataset_ ##name)/sizeof(chaos_ds_t));

/**
 * Initialize the API context
 * @param chaoswanurl the chaoswan proxy url
 * @return the handle, 0 otherwise
 */
chaos_crest_handle_t chaos_crest_open(const char* chaoswanurl);

/**
 * connect to the server
 * @param h api handle
 * @return 0 on success
 */

int chaos_crest_connect(chaos_crest_handle_t h);
/**
 * create a cu of a given name with a specificated number of input and output dataset
 * @param h api handle
 * @param dsin CU dataset array
 * @param dsitems number of elements
 * @return an UID of the created CU, negative if error
 */
uint32_t chaos_crest_add_cu(chaos_crest_handle_t h,const char*name,chaos_ds_t* ds,int dsitems);

/**
 * update a dataset value with a new value
 * @param h api handle
 * @param cu_uid the UID associated to the CU
 * @param attr_pos the index on the output dataset vector that we want modify
 * @param data pointer to the data to transmit
 * @return 0 on success, error otherwise
 */

int chaos_crest_update(chaos_crest_handle_t h,uint32_t cu_uid,int attr_pos,void*data);

/**
 * update a dataset value with a new value, by indexing attribute by name
 * @param h api handle
 * @param cu_uid the UID associated to the CU
 * @param attr_name the name of the output dataset vector that we want modify
 * @param data pointer to the data to transmit
 * @return 0 on success, error otherwise
 */
int chaos_crest_update_by_name(chaos_crest_handle_t h,uint32_t cu_uid,char* attr_name,void*data);

/**
 * registers the existing CUs
 * @param h api handle
 * @param cu_uid the cu_uid that we want to push on the cloud, 0 means all the cu defined
 * @return 0 on success, error otherwise
 */
int chaos_crest_register(chaos_crest_handle_t h,uint32_t cu_uid);
/**
 * push the dataset of the specified CU on the cloud
 * @param h api handle
 * @param cu_uid the cu_uid that we want to push on the cloud, 0 means all the cu defined
 * @return 0 on success, error otherwise
 */
int chaos_crest_push(chaos_crest_handle_t h,uint32_t cu_uid);

/**
 * return the average push time in ms
 * @param h api handle
 * @return the average push time in ms, 0 if NA
 */
float chaos_crest_push_time(chaos_crest_handle_t h);


/**
 * return the average registration time in ms
 * @param h api handle
 * @return the average registration time in ms, 0 if NA
 */
float chaos_crest_reg_time(chaos_crest_handle_t h);

/**
 * Send a command to a CU
 * @param h api handle
 * @param cuname name cu name to access
 * @param cmd command to sent (i.e. init, deinit, start, stop)
 * @param arg arguments, 0 if none
 * @return 0 on success, otherwise error
 */

int chaos_crest_cu_cmd(chaos_crest_handle_t h,const char*cuname,const char*cmd,const char* args);

/**
 * Retrieve the output dataset of a given CU
 * @param h api handle
 * @param cuname name cu name to access
 * @param [out] output buffer where the dataset is written out
 * @param maxsize max size of the buffer
 * @return timestamp in ms on success, 0 if error
 */

uint64_t chaos_crest_cu_get(chaos_crest_handle_t h,const char*cuname,char*output,int maxsize);

/**
 * Retrieve the value of an output channel of a given CU
 * @param h api handle
 * @param cuname name cu name to access
 * @param channame name of the channel
 * @param [out] output buffer where the value is written out
 * @param maxsize max size of the buffer
 * @return timestamp in ms on success, 0 if error
 */

uint64_t chaos_crest_cu_get_channel(chaos_crest_handle_t h,const char*cuname,const char*channame,char*output,int maxsize);

/**
 * Close the API context and free resources
 * @param h api handle
 * @return 0 on success, otherwise error
 */

int chaos_crest_close(chaos_crest_handle_t h);
#endif
