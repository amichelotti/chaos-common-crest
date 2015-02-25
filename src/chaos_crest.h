// chaos_crest.h
// @author Andrea Michelotti
// Simple apis to access chaos using http rest
#ifndef _CHAOS_CREST_H_
#define _CHAOS_CREST_H_

typedef void* chaos_crest_handle_t;

typedef struct _ds{
    const char *name;
    const char *desc;
    int        type;
    int        size;
} chaos_ds_t;
/**
    Initialize the API context
    @param chaoswan the chaoswan proxy
    @return the handle, 0 otherwise
 */
chaos_crest_handle_t chaos_crest_open(const char* chaoswan);

/**
 connect to the server
 @param h api handle
 @return 0 on success
 */

int chaos_crest_connect(chaos_crest_handle_t h);
/**
 create a cu of a given name with a specificated number of input and output dataset
 @param h api handle
 @param dsin array of chaos datasets in input
 @param ndsin number of input datasets
 @param dsout array of chaos datasets in output
 @param ndsout number of output datasets
 @return an UID of the created CU, negative if error
 */
int chaos_crest_add_cu(chaos_crest_handle_t h,const char*name,chaos_ds_t* dsin,int ndsin,chaos_ds_t* dsout,int ndsout);

/**
 update a dataset value with a new value
 @param h api handle
 @param cu_uid the UID associated to the CU
 @param attr_pos the index on the output dataset vector that we want modify
 @param data pointer to the data to transmit
 @return 0 on success, error otherwise
 */

int chaos_crest_update(chaos_crest_handle_t h,int cu_uid,int attr_pos,void*data);

/**
 update a dataset value with a new value, by indexing attribute by name
 @param h api handle
 @param cu_uid the UID associated to the CU
 @param attr_name the name of the output dataset vector that we want modify
 @param data pointer to the data to transmit
 @return 0 on success, error otherwise
 */
int chaos_crest_update_by_name(chaos_crest_handle_t h,int cu_uid,char* attr_name,void*data);

/**
 registers the existing CUs
 @param h api handle
  @return 0 on success, error otherwise
 */
int chaos_crest_register(chaos_crest_handle_t h);
/**
 push the dataset of the specified CU on the cloud
 @param h api handle
@param cu_uid the cu_uid that we want to push on the cloud, 0 means all the cu defined
 @return 0 on success, error otherwise
 */
int chaos_crest_push(chaos_crest_handle_t h,int cu_uid);
#endif