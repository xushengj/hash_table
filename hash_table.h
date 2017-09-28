/*
 * Simple generic hash table in C
 * Same key for different values is not allowed
 * Channing is used when hash colliding
 *
 * To use this hash table:
 * 1.	use HASH_TABLE_DECLARE macro in a header file
 *		HASH_TABLE_DECLARE parameter:
 *		ht_type		type name for this hash table
 *					normal rules for type name apply
 *					functions for this hash table will have names starting
 *					with this prefix
 *					you can also use this type name to create instances
 *					all function calls accessing instances of this hash table
 *					need to pass this name as first parameter
 *
 *		key_type	type name for key
 *					keys will be copied using '=' operator
 *
 *		value_type	type name for value
 *					values will also be copied using '=' operator
 *
 *		hash_func	hash function
 *					the macro expects "hash_func(key)" to return an unsigned
 *					integer value (e.g. size_t)
 *					if you want to use key type as hash value directly,
 *					please pass HASH_TABLE_DEFAULT_HASH
 *
 *		pred_func	predicate function
 *					the macro expects "pred_func(key1,key2)" to return non-zero
 *					when key1 and key2 are considered the same, and zero if not
 *					if key can be compared using "==" operator, please pass
 *					HASH_TABLE_DEFAULT_PRED
 *
 *		malloc_func	malloc like function
 *					i.e. void* malloc_func(size_t size);
 *
 *		free_func	free like function
 *					i.e. void free_func(void* ptr);
 *
 *
 *		The following fields are optional(use HASH_TABLE_DECLARE7):
 *
 *		min_bin_size_val	minimum & initial number of bins/buckets
 *
 *		aim_ldfac_val		goal load factor
 *							ht_rehash use this value to get number of bins
 *
 *		max_ldfac_val		maximum load factor
 *							after a successful insertion, if this load factor
 *							is exceeded, rehash is attempted before returning
 *
 *		min_inv_ldfac_val	reciprocal of minimum load factor
 *							after a successful erase, if load factor is less
 *							than reciproal of this value, then rehash is
 *							attempted before returning
 *
 *	2.	use HASH_TABLE_DEFINE macro in a source file
 *		all parameters should match with HASH_TABLE_DECLARE
 *
 *	Now you can use the hash table anywhere the declaration is visible.
 *
 *	Folowing functions are provided for hash table:
 *
 *	void ht_init(ht_type, ht_type* ht_ptr);
 *			initialize states in hash table
 *			(purely macro)
 *
 *	void ht_clear(ht_type, ht_type* ht_ptr);
 *			clear the hash table
 *
 *	size_t ht_size(ht_type, ht_type* ht_ptr);
 *			get number of items in table
 *			(purely macro)
 *
 *	value_type* ht_find(ht_type, ht_type* ht_ptr, key_type key);
 *			try to find the pair with given key
 *			return NULL if pair not found
 *			return pointer to value if pair is found
 *			In current implementation, the item will be in the same position
 *			in memory after it is inserted and before it is erased
 *
 *	value_type* ht_insert(ht_type, ht_type* ht_ptr, key_type key, value_type value);
 *			try to insert (key,value) pair
 *			return result of ht_find after insertion, NULL if insertion failed
 *
 *	int ht_erase(ht_type, ht_type* ht_ptr, key_type key);
 *			try to erase pair with given key
 *			return 1 on success, 0 on failure
 *
 *	void ht_rehash(ht_type, ht_type* ht_ptr);
 *			try to adjust number of bins/buckets to achieve aim load factor
 *
 *	struct detail:
 *	ht_type will be a struct with following members:
 *		size_t itemCount				number of pairs in the hash table
 *		size_t binCount					number of bins/buckets
 *		struct ht_type##_bin* binVec	pointer to (malloc'd) array of bins
 *
 *	ht_type##_bin is a struct representing a bin/bucket. It has only one member
 *		struct ht_type##_item* first	pointer to first item in the bin
 *
 *	ht_type##_item is a struct representing an item (pair) in the table.
 *		key_type key
 *		value_type value
 *		struct ht_type##_item* next		pointer to next item in current bin
 */

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/* direct inline */
#define ht_init( ht_type , ht_ptr)\
do{\
	ht_ptr->itemCount=0;\
	ht_ptr->binCount=0;\
	ht_ptr->binVec=NULL;\
}while(0)

#define ht_size( ht_type , ht_ptr) (ht_ptr->itemCount)


/* dispatch according to type */
#define ht_clear( ht_type , ht_ptr)					ht_type##_ht_clear(ht_ptr)
#define ht_find( ht_type , ht_ptr, key)				ht_type##_ht_find(ht_ptr, key)
#define ht_insert( ht_type , ht_ptr, key, value)	ht_type##_ht_insert(ht_ptr, key, value)
#define ht_erase( ht_type , ht_ptr, key)			ht_type##_ht_erase(ht_ptr, key)
#define ht_rehash( ht_type , ht_ptr)				ht_type##_ht_rehash(ht_ptr)


#define HASH_TABLE_PRIVATE_SIGN_ht_clear( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
void ht_type##_ht_clear( ht_type * ht_ptr)
#define HASH_TABLE_PRIVATE_IMPL_ht_clear( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
{\
	if(ht_ptr->binVec!=NULL){\
		struct ht_type##_item* tmp;\
		struct ht_type##_item* next;\
		size_t i;\
		for(i=0;i<ht_ptr->binCount;++i){\
			tmp=ht_ptr->binVec[i].first;\
			ht_ptr->binVec[i].first=NULL;\
			while(tmp!=NULL){\
				next=tmp->next;\
				free_func(tmp);\
				tmp=next;\
			}\
		}\
		free_func(ht_ptr->binVec);\
	}\
	ht_ptr->itemCount=0;\
	ht_ptr->binCount=0;\
	ht_ptr->binVec=NULL;\
}

#define HASH_TABLE_PRIVATE_SIGN_ht_find( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
value_type* ht_type##_ht_find( ht_type * ht_ptr, key_type key)
#define HASH_TABLE_PRIVATE_IMPL_ht_find( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
{\
	if((ht_ptr->itemCount>0)&&(ht_ptr->binCount>0)){\
		struct ht_type##_bin* bin=&(ht_ptr->binVec[(hash_func(key))%(ht_ptr->binCount)]);\
		if(bin->first!=NULL){\
			struct ht_type##_item* item=bin->first;\
			while(item!=NULL){\
				if(pred_func(key,item->key)){\
					return &(item->value);\
				}\
				item=item->next;\
			}\
		}\
	}\
	return NULL;\
}

#define HASH_TABLE_PRIVATE_SIGN_ht_insert( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
value_type* ht_type##_ht_insert( ht_type * ht_ptr, key_type key, value_type value)
#define HASH_TABLE_PRIVATE_IMPL_ht_insert( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
{\
	struct ht_type##_bin* binPtr;\
	struct ht_type##_item* itemPtr;\
	struct ht_type##_item** insertPtr;\
	\
	if(ht_ptr->binVec==NULL){\
		size_t i;\
		\
		ht_ptr->binVec=malloc_func(sizeof(struct ht_type##_bin)*(min_bin_size_val));\
		if(ht_ptr->binVec==NULL){\
			return NULL;\
		}\
		ht_ptr->binCount=(min_bin_size_val);\
		for(i=0;i<(min_bin_size_val);++i){\
			ht_ptr->binVec[i].first=NULL;\
		}\
		ht_ptr->itemCount=0;\
	}\
	\
	binPtr=&(ht_ptr->binVec[(hash_func(key))%(ht_ptr->binCount)]);\
	if(binPtr->first!=NULL){\
		for(itemPtr=binPtr->first;\
			itemPtr->next!=NULL;\
			itemPtr=itemPtr->next){\
			\
			if(pred_func(itemPtr->key,key)){\
				return &(itemPtr->value);\
			}\
		}\
		\
		if(pred_func(itemPtr->key,key)){\
			return &(itemPtr->value);\
		}\
		insertPtr=&(itemPtr->next);\
	}else{\
		insertPtr=&(binPtr->first);\
	}\
	\
	itemPtr=malloc_func(sizeof(struct ht_type##_item));\
	if(itemPtr==NULL){\
		return NULL;\
	}\
	\
	itemPtr->key=key;\
	itemPtr->value=value;\
	itemPtr->next=NULL;\
	*(insertPtr)=itemPtr;\
	ht_ptr->itemCount+=1;\
	if((ht_ptr->itemCount)/(ht_ptr->binCount)>(max_ldfac_val)){\
		ht_rehash(ht_type , ht_ptr);\
	}\
	return &(itemPtr->value);\
}

#define HASH_TABLE_PRIVATE_SIGN_ht_erase( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
int ht_type##_ht_erase( ht_type * ht_ptr, key_type key)
#define HASH_TABLE_PRIVATE_IMPL_ht_erase( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
{\
	if((ht_ptr->itemCount>0)&&(ht_ptr->binCount>0)){\
		struct ht_type##_bin* bin=&(ht_ptr->binVec[hash_func(key)%(ht_ptr->binCount)]);\
		if(bin->first!=NULL){\
			struct ht_type##_item* item=bin->first;\
			struct ht_type##_item** removePtr=&(bin->first);\
			while(item!=NULL){\
				if(pred_func(key,item->key)){\
					(*removePtr)=item->next;\
					free_func(item);\
					ht_ptr->itemCount-=1;\
					if((ht_ptr->itemCount>0)&&((ht_ptr->binCount)/(ht_ptr->itemCount)>(min_inv_ldfac_val))){\
						ht_rehash(ht_type , ht_ptr);\
					}\
					return 1;\
				}\
				removePtr=&(item->next);\
				item=item->next;\
			}\
		}\
	}\
	return 0;\
}

#define HASH_TABLE_PRIVATE_SIGN_ht_rehash( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
void ht_type##_ht_rehash( ht_type * ht_ptr)
#define HASH_TABLE_PRIVATE_IMPL_ht_rehash( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
{\
	size_t i;\
	size_t aimBinCount=(ht_ptr->itemCount)/(aim_ldfac_val);\
	if(aimBinCount<(min_bin_size_val)){\
		aimBinCount=(min_bin_size_val);\
	}\
	if(aimBinCount!=ht_ptr->binCount){\
		struct ht_type##_bin* newBinVec=malloc_func(sizeof(struct ht_type##_bin)*aimBinCount);\
		if(newBinVec==NULL){\
			return;\
		}\
		for(i=0;i<aimBinCount;++i){\
			newBinVec[i].first=NULL;\
		}\
		if(ht_ptr->itemCount>0){\
			for(i=0;i<ht_ptr->binCount;++i){\
				struct ht_type##_item* item=ht_ptr->binVec[i].first;\
				while(item!=NULL){\
					struct ht_type##_item* next;\
					struct ht_type##_bin* bin;\
					\
					bin=&(newBinVec[hash_func(item->key)%aimBinCount]);\
					next=item->next;\
					item->next=bin->first;\
					bin->first=item;\
					item=next;\
				}\
			}\
		}\
		free_func(ht_ptr->binVec);\
		ht_ptr->binVec=newBinVec;\
		ht_ptr->binCount=aimBinCount;\
	}\
}

#define HASH_TABLE_DECLARE(\
	ht_type,\
	key_type,\
	value_type,\
	hash_func,\
	pred_func,\
	malloc_func,\
	free_func,\
	min_bin_size_val,\
	aim_ldfac_val,\
	max_ldfac_val,\
	min_inv_ldfac_val\
)\
\
struct ht_type##_bin;\
typedef struct{\
	size_t itemCount;\
	size_t binCount;\
	struct ht_type##_bin* binVec;\
}ht_type;\
\
HASH_TABLE_PRIVATE_SIGN_ht_clear( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val);\
HASH_TABLE_PRIVATE_SIGN_ht_find( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val);\
HASH_TABLE_PRIVATE_SIGN_ht_insert( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val);\
HASH_TABLE_PRIVATE_SIGN_ht_erase( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val);\
HASH_TABLE_PRIVATE_SIGN_ht_rehash( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val);

#define HASH_TABLE_DEFINE(\
	ht_type,\
	key_type,\
	value_type,\
	hash_func,\
	pred_func,\
	malloc_func,\
	free_func,\
	min_bin_size_val,\
	aim_ldfac_val,\
	max_ldfac_val,\
	min_inv_ldfac_val\
)\
\
struct ht_type##_item{\
	key_type key;\
	value_type value;\
	struct ht_type##_item* next;\
};\
\
struct ht_type##_bin{\
	struct ht_type##_item* first;\
};\
\
HASH_TABLE_PRIVATE_SIGN_ht_clear( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_IMPL_ht_clear( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_SIGN_ht_find( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_IMPL_ht_find( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_SIGN_ht_insert( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_IMPL_ht_insert( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_SIGN_ht_erase( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_IMPL_ht_erase( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_SIGN_ht_rehash( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)\
HASH_TABLE_PRIVATE_IMPL_ht_rehash( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, min_bin_size_val, aim_ldfac_val, max_ldfac_val, min_inv_ldfac_val)

/* use as hash_func if key is unique integer id */
#define HASH_TABLE_DEFAULT_HASH(key) (key)

/* use as pred_func if it is okay to use equal sign comparison */
#define HASH_TABLE_DEFAULT_PRED(key1, key2) ((key1)==(key2))

/* if you do not care about details */
#define HASH_TABLE_DECLARE7(\
	ht_type,\
	key_type,\
	value_type,\
	hash_func,\
	pred_func,\
	malloc_func,\
	free_func\
)\
HASH_TABLE_DECLARE( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, 32, 2, 16, 16)

#define HASH_TABLE_DEFINE7(\
	ht_type,\
	key_type,\
	value_type,\
	hash_func,\
	pred_func,\
	malloc_func,\
	free_func\
)\
HASH_TABLE_DEFINE( ht_type , key_type, value_type, hash_func, pred_func, malloc_func, free_func, 32, 2, 16, 16)

#endif /* HASH_TABLE_H */