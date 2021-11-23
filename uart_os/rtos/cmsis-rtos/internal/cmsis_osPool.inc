/*
 *  $HeadURL: $
 *
 *  $Date: $
 *  $Author: $
 */


/* \file
 * \brief
 * Implements the cmsis RTOS memory pool API.
 */

#include "build-config.h" /* This must be the first include. It will pull the TARTGET_RTOS macro. */

#if TARGET_RTOS != RTOS_ThreadX /* For ThreadX we uses its own pool implementation. */

osPoolId osPoolCreate (const osPoolDef_t *pool_def){
  return os_pool_cb::create(pool_def->pool_sz, pool_def->item_sz);
}

void *osPoolAlloc (osPoolId pool_id){
  osAssertHandle(pool_id);
  return pool_id->alloc(0);
}

void *osPoolCAlloc (osPoolId pool_id){
  osAssertHandle(pool_id);
  return pool_id->calloc(0);
}

osStatus osPoolFree (osPoolId pool_id, void *block){
  osAssertHandle(pool_id);
  return pool_id->freeBlock(block);
}

osStatus osPoolDestroy_suppl(osPoolId* pool_id_ptr, uint32_t millisec) {
  typedef _os::MemoryPoolDestructor<os_pool_cb> destructor;
  return destructor::poolDestroy(pool_id_ptr, millisec);
}

uint32_t osPoolBlockSizeGet_suppl(osPoolId pool_id) {
  return pool_id->blockSize();
}
#endif /* #if TARGET_RTOS != RTOS_ThreadX  */
