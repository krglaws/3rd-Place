#ifndef _STRING_MAP_H_
#define _STRING_MAP_H_

/* This header includes some useful functions for parsing
 * strings into ks_hashmaps, and also for retrieving values
 * by key from ks_hashmaps.
 */

#include <kylestructs.h>


/* string_to_map():
 * Converts the contents of a string into a ks_hashmap object. 
 * char* str - string to read into ks_hashmap
 * const char* delim1 - delimiter separating pairs
 * const char* delim2 - delimiter separating key-value
 *
 * returns ks_hashmap*
 */
ks_hashmap* string_to_map(char* str, const char* delim1, const char* delim2);


/* get_map_value():
 * Retrieves a value from a map with a key string.
 * const ks_hashmap* map - the map from which to retrieve a value
 * const char* keystr - the key used to retrieve a value from 'map'
 *
 * returns const ks_datacont*
 */
const ks_datacont* get_map_value(const ks_hashmap* map, const char* keystr);


/* Various helper functions for adding values to ks_hashmap
 */
void add_map_value_str(ks_hashmap* hm, const char* key, const char* val);
void add_map_value_ls(ks_hashmap* hm, const char* key, const ks_list* val);
void add_map_value_hm(ks_hashmap* hm, const char* key, const ks_hashmap* val);

#endif
