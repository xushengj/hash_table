#include "hash_table.h"
#include <stdlib.h>

HASH_TABLE_DECLARE7( htest, size_t, size_t, HASH_TABLE_DEFAULT_HASH, HASH_TABLE_DEFAULT_PRED, malloc, free)

HASH_TABLE_DEFINE7( htest, size_t, size_t, HASH_TABLE_DEFAULT_HASH, HASH_TABLE_DEFAULT_PRED, malloc, free)