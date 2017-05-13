/* mempool.h -- Fixed size memory pool.
 *              Uses system allocation if the pool is full.
 *              To use start with a call to mp_create.
 */


void  mp_create(size_t size_of_block, size_t nr_of_blocks);
void* mp_alloc(void);
void  mp_free(void* block);
