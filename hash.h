//this file contains code from the ht repository by Ben Hoyt, which is licensed under the MIT license

#ifndef _HASH_H
#define _HASH_H

#include <stdbool.h>
#include <stddef.h>

// Hash table structure: create with ht_create, free with ht_destroy.
/*typedef struct ht ht;

typedef struct ht_subentry ht_subentry;*/

// Hash table entry (slot may be filled or empty).

// Hash table structure: create with ht_create, free with ht_destroy.

typedef struct ht ht;
typedef struct ht_subentry ht_subentry;
typedef struct ht_subtable ht_subtable;
typedef struct ht_entry ht_entry;

// Create hash table and return pointer to it, or NULL if out of memory.
ht* ht_create(size_t capacity);

// Free memory allocated for hash table, including allocated keys.
//void ht_destroy(ht* table);

// Get item with given key (NUL-terminated) from hash table. Return
// value (which was set with ht_set), or NULL if key not found.
//void* ht_get(ht* table, const char* key);


// custom, document later (TODO)
size_t get_bucket_index(void* addr, bool shared);


ht* ht_create(size_t capacity);

ht_subtable* ht_subtable_create(size_t capacity);

void ht_add_entry(ht* table, const char* key, void* value, int uid);

void ht_remove_entry(ht* table, const char* key, int uid);

ht_subentry* get_subentry(ht* table, const char* key, int uid);



// Set item with given key (NUL-terminated) to value (which must not
// be NULL). If not already present in table, key is copied to newly
// allocated memory (keys are freed automatically when ht_destroy is
// called). Return address of copied key, or NULL if out of memory.
//const char* ht_set(ht* table, const char* key, void* value);

// Return number of items in hash table.
size_t ht_length(ht* table);

// Hash table iterator: create with ht_iterator, iterate with ht_next.
typedef struct {
    const char* key;  // current key
    void* value;      // current value

    // Don't use these fields directly.
    ht* _table;       // reference to hash table being iterated
    size_t _index;    // current index into ht._entries
} hti;

// Return new hash table iterator (for use with ht_next).
hti ht_iterator(ht* table);

// Move iterator to next item in hash table, update iterator's key
// and value to current item, and return true. If there are no more
// items, return false. Don't call ht_set during iteration.
//bool ht_next(hti* it);

#endif // _HASH_H