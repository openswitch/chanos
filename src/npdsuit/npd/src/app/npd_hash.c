/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		hash table calculation routines.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.16 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"

#include "npd_log.h"
#include "npd_hash.h"

pthread_mutex_t sharedMutex=PTHREAD_MUTEX_INITIALIZER;
/* Allocate new hash. */
struct Hash * hash_new 
(
	int size 
)
{
    struct Hash *node = NULL;
	
    node = (struct Hash*)malloc(sizeof(struct Hash));
    if ( !node )
    {
        return NULL;
    }
    memset(( char * ) node,0,sizeof(struct Hash));

    node->index = malloc(sizeof(struct hash_bucket *)*size);	
    if (!node->index)
    {
		free(node);
        return NULL;
    }
	node->index2 = malloc (sizeof(struct hash_bucket *)*size);
    if (!node->index2)
    {
        free(node->index);
		free(node);
        return NULL;
    }
    
    memset((char *)node->index,0,sizeof(struct hash_bucket*)*size);
	memset((char *)node->index2,0,sizeof(struct hash_bucket*)*size);
    node->hash_cmp = NULL;
	node->hash_key = NULL;
	node->hash_key2 = NULL;
    node->size = size;

    return node;
}

/* allocate new hash bucket */
struct hash_bucket * hash_backet_new
( 
	void *data
)
{
    struct hash_bucket * bucket = NULL;

    bucket = malloc(sizeof(struct hash_bucket));
    if ( !bucket )
    {
        return NULL;
    }
    bucket->data = data;
    bucket->next = NULL;
	bucket->next2 = NULL;
    
    return bucket;
}

struct hash_bucket  *hash_head
( 
	struct Hash *hash, 
	int index ,
	int headType
)
{
	if(NULL == hash){
		return NULL;
	}
    if(!headType){
        return hash->index[index];
    }
	else {
		return hash->index2[index];
	}
}

/* Hash search */
void *hash_search 
( 
	struct Hash *hash, 
	void *data,
	int (*compFunc)(void*,void*),
	int keyType
)
{
    unsigned int    key;
    struct hash_bucket *bucket = NULL;
	struct hash_bucket ** index = NULL;
	if((NULL == hash)||(NULL == data)){
        return NULL;
	}
    if(!keyType){
	    key = (*hash->hash_key)(data);
		index = hash->index;
    }
	else {
		key = (*hash->hash_key2)(data);
		index = hash->index2;
	}
	if(key >= hash->size){
        return NULL;
	}
    if (index[ key ] == NULL)
    	return NULL;
            
	for(bucket = index[key]; bucket != NULL; ) {
		if(NULL!= compFunc) {
			if((*compFunc)(bucket->data, data) == TRUE)
				return bucket->data;
		}
		else {
			if((*hash->hash_cmp)(bucket->data,data) == TRUE)
				return bucket->data;
		}
		if(!keyType){/* for key1*/
			bucket = bucket->next;
		}
		else{/* for key2*/
			bucket = bucket->next2;
		}
	}
	
    return NULL;
}

/* Push data into hash. */
void * hash_push
( 
	struct Hash *hash, 
	void *data
)
{
    unsigned int    key1 = 0,key2 = 0;
    struct hash_bucket *bucket = NULL,*mp = NULL;
	struct hash_bucket ** index1 = NULL;
	struct hash_bucket ** index2 = NULL;
	int * count = NULL;	
    if((NULL == hash)||(NULL == data)){
		return NULL;
    }

	/*lock mutex*/
	pthread_mutex_lock(&sharedMutex);	
	count = &(hash->count);
	
    bucket = hash_backet_new ( data );/* get a new node*/
    
    (*count)++;
    /* for key1*/
	if(NULL != hash->hash_key){
        key1 = ( *hash->hash_key ) ( data );
	    index1 = hash->index;
		if(key1 >= hash->size){
	        return NULL;
		}
	    if ( index1[ key1 ] == NULL )
	    {
	        index1[ key1 ]=bucket;
	    }
	    else
	    {
	        for (mp = index1[ key1 ];mp->next != NULL;mp = mp->next)
	        {
	            if (( *hash->hash_cmp )( data, mp->data) == TRUE)
	            {
	                free(bucket);
	                (*count)--; 
									
					/*unlock mutex*/
					pthread_mutex_unlock(&sharedMutex);
	                return NULL;
	            }
	        } /* FOR (...) */
	        
	        if ((*hash->hash_cmp)( data, mp->data) == TRUE )
	        {
	            free(bucket);
	            (*count)--;

				/*unlock mutex*/
				pthread_mutex_unlock(&sharedMutex);
	            return NULL;
	        }
	        
	        mp->next = bucket;
	    }
	}
	if(NULL != hash->hash_key2){
	    /* for key2 */	
	    key2 = ( *hash->hash_key2 ) ( data );
	    index2 = hash->index2;	
		if(key2 >= hash->size){
	        return NULL;
		}
	    if ( index2[ key2 ] == NULL )
	    {
	        index2[ key2 ]=bucket;
	    }
	    else
	    {
	        for (mp = index2[ key2 ];mp->next2 != NULL;mp = mp->next2)
	        {
	            if (( *hash->hash_cmp )( data, mp->data) == TRUE)
	            {
	                free(bucket);
	                (*count)--; 
									
					/*unlock mutex*/
					pthread_mutex_unlock(&sharedMutex);
	                return NULL;
	            }
	        } /* FOR (...) */
	        
	        if ((*hash->hash_cmp)( data, mp->data) == TRUE )
	        {
	            free(bucket);
	            (*count)--;

				/*unlock mutex*/
				pthread_mutex_unlock(&sharedMutex);
	            return NULL;
	        }
	        
	        mp->next2 = bucket;
	    } 
	}
	if((!(hash->hash_key))&&(!(hash->hash_key2))){
		free(bucket);
		(*count)--;
    	pthread_mutex_unlock(&sharedMutex);
	    return NULL;
	}
	/*unlock mutex*/
	pthread_mutex_unlock(&sharedMutex);
    return bucket->data;
}


/* When deletion is finished successfully return data of delete
   backet. */
void *hash_pull 
( 
	struct Hash *hash, 
	void *data 
)
{
    unsigned int    key1 = 0,key2 = 0;
    void  *ret = NULL;
    struct hash_bucket *mp1  = NULL;
    struct hash_bucket *mp2  = NULL;
    struct hash_bucket *mpp = NULL;
	struct hash_bucket ** index1 = NULL;
	struct hash_bucket ** index2 = NULL;
	int * count = NULL;
	if((NULL == hash)||(NULL == data)){
		return NULL;
	}
	
	index1 = hash->index;
	index2 = hash->index2;	

	/*lock mutex*/
	pthread_mutex_lock(&sharedMutex);	
	count = &(hash->count);	
	if(hash->hash_key){/* pull for key1 if not null*/

		key1 = (*hash->hash_key)(data);
		if(key1 >= hash->size){
			return NULL;
		}
		if (index1[key1] == NULL ){
	        return NULL;
		}
	    mp1 = mpp = index1[ key1 ];
	    while ( mp1 )
	    {
	        if (( *hash->hash_cmp)(mp1->data,data) == TRUE)
	        {
	            if ( mp1==mpp)
	            {
	                index1[ key1 ] = mp1->next;
	            }
	            else
	            {
	                mpp->next = mp1->next;
	            }
	            /*ret = mp1->data;*/   
				mpp = NULL;
	            break;
	        }	        
	        mpp = mp1;
	        mp1 = mp1->next;
	    } /* WHILE (...) */
	}
	if(hash->hash_key2){ /* pull for key2 if not null*/

		key2 = (*hash->hash_key2)(data);
		if(key1 >= hash->size){
			return NULL;
		}
		if (index2[key2] == NULL ){
	        return NULL;
		}
		mp2 = mpp = index2[ key2 ];
		while ( mp2 )
		{
			if (( *hash->hash_cmp)(mp2->data,data) == TRUE)
			{
				if ( mp2==mpp)
				{
					index2[ key2 ] = mp2->next2;
				}
				else
				{
					mpp->next2 = mp2->next2;
				}
				/*if(NULL == ret){
				   // ret = mp2->data;
				//}*/
				mpp = NULL;
	            break;
			}
			mpp = mp2;
			mp2 = mp2->next2;
			
		} /* WHILE (...) */
	}
	if((NULL != mp1)&&(mp1 == mp2)){           /* both have key1 and key2,and get the node*/
	    ret = mp1->data;
	    free(mp1);
		mp1 = NULL;
		mp2 = NULL;
	}
	else if((NULL != mp1)&&(!(hash->hash_key2))){ /* only have key1,and get the node */
		    ret = mp1->data;
			free(mp1);
			mp1 = NULL;
	}
	else if((NULL != mp2)&&(!(hash->hash_key))){ /* only have key2,and get the node*/
		    ret = mp2->data;
			free(mp2);
			mp2 = NULL;
	}
	else{
		/* didn't get the node*/
		/*unlock mutex*/
		pthread_mutex_unlock(&sharedMutex);	    
	    return NULL;
	}
	(*count)--;
	pthread_mutex_unlock(&sharedMutex);	    
    return ret;
}


void hash_clean 
( 
	struct Hash *hash, 
	void (* func)( void *) 
)
{
    int         i;
    struct hash_bucket *mp = NULL;
    struct hash_bucket *next = NULL;
	void * data = NULL;
    if(NULL == hash){
		return ;
    }
    for ( i = 0; i < hash->size; i++ )
    {
        for ( mp = hash_head ( hash, i ,0); mp; mp = next )
        {
            next = mp->next;            
            data = hash_pull(hash,mp->data);
            if (func&&data)
            {
                (*func)(data);
            }
            /*free(mp);*/
        } /* FOR (...) */
        
        hash->index[ i ] = NULL;
    } /* FOR (...) */

    hash->count = 0;
}

void hash_free 
( 
	struct Hash *hash, 
	void (* func)(void *)
)
{
	if(NULL == hash){
		return ;
    }
    hash_clean ( hash, func );
    free(hash->index );
	free(hash->index2 );
    free(hash);
}

/*
  * @return hash table count
  *
  */
int hash_count
(
	struct Hash *hash
)
{
	if(NULL == hash) 
		return 0;
	else {
		return hash->count; 
	}
}

void hash_show
(
	struct Hash *hash,
	void (*showFunc)(void *,unsigned char)
)
{
	int i = 0,count = 0;
	unsigned char withTitle = 0;
	struct hash_bucket *backet = NULL, *next = NULL;
    if(NULL == hash){
		return ;
    }

	if(NULL == showFunc) return;
	
	for(i=0;i<hash->size;i++) {
		for(backet = hash->index[i]	;backet;backet=next) {
			next = backet->next;
			if(0 == count) {
				withTitle = TRUE;
			}
			else {
				withTitle = FALSE;
			}
			if(NULL != showFunc) {
				(*showFunc)(backet->data,withTitle);
			}
			count++;
		}
	}
}

/*
  * traversal hash table to get hash item by the specified filter,
  * and then process matched item as the processor requires.
  * If no filter specified, all hash items are relevant to processor.
  * If no processor specified, all filtered items are counted.
  *
  * PARAMETERS:
  *		data - data used by filter to pick hash items
  *		filter - filter function to filter out irrelevant items, 
  *			    function has two input arguments:
  *				first argument is the hash item need to filter
  *				second argument is data used to filter to.
  *		processor - trigger function called when filter matched,
  *			    function has one input arguments:
  *				input argument is the hash item
  */
unsigned int hash_traversal
(
	struct Hash *hash,
	unsigned int flag,
	void * data,
	unsigned int (*filter)(void *,void *),
	int (*processor)(void *,unsigned int)
)
{
	int i = 0,count = 0;
	unsigned int filterMatch = 0;
	struct hash_bucket *backet = NULL, *next = NULL;
	if(NULL == hash){
		return 0;
	}
	for(i=0;i<hash->size;i++) {
		for(backet = hash->index[i]	;backet;backet=next) {
			next = backet->next;
			if(NULL != filter) { /* specific filter given*/
				filterMatch = (*filter)(backet->data,data);				
			}
			else { /* no filter specified*/
				filterMatch = TRUE;
			}

			if(TRUE == filterMatch) count++;
			
			if((TRUE == filterMatch) && (NULL != processor)) {
				(*processor)(backet->data,flag);
			}
		}
	}

	return count;
}


/*
  * by zhubo@autelan.com show hash item by port
  * get hash table item by the specified filter,
  * If no filter specified, all hash items return.
  *
  * PARAMETERS:
  *		indata - data used by filter to pick hash items
  *		outdata - data return to user
  *		filter - filter function to filter out irrelevant items, 
  *			    function has two input arguments:
  *				first argument is the hash item need to filter
  *				second argument is data used to filter to.
  */
void hash_return
(
	struct Hash *hash,
	void * indata,
	void * outdata[],
	unsigned int len,
	unsigned int (*filter)(void *,void *)	
)
{
	int i = 0,count = 0;
	struct hash_bucket *backet = NULL, *next = NULL;
	unsigned int filterMatch = 0;
    if(NULL == hash){
		return ;
    }
	
	for(i=0;i<hash->size;i++) {
		for(backet = hash->index[i]	;backet;backet=next) {
			next = backet->next;
			if(NULL != filter) { /* specific filter given*/
				filterMatch = (*filter)(backet->data,indata);				
			}
			else { /* no filter specified*/
				filterMatch = TRUE;
			}

			if(TRUE == filterMatch) {
				if(count < len ) {
					outdata[count] = backet->data;
					/*printf("outdata[%d] %p, backet->data %p\n",count,outdata[count],backet->data);*/
				}
				
				count++;
			}
		}
	}

}

#ifdef __cplusplus
}
#endif
