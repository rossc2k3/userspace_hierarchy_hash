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

// // Hash table entry (slot may be filled or empty).
// typedef struct {
//     const char* key;  // key is NULL if this slot is empty
//     void* value;
// } ht_subentry;

typedef struct {
    const char* key;
    void* value;
} ht_entry_item;

typedef struct {
    ht_entry_item* items;
    size_t count;
    size_t capacity;
} ht_subentry_list;

typedef struct {
    ht_subentry_list* entries;
} ht_subentry;

typedef struct {
    ht_subentry* entries;  // hash slots
    size_t capacity;       // size of _entries array
    size_t length;         // number of items in hash table
} ht_subtable;

typedef struct {
    ht_subtable* subtable;
} ht_entry;

typedef struct {
    ht_entry* entries;  // hash slots
    size_t capacity;    // size of _entries array
    size_t length;      // number of items in hash table
} ht;

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

int ht_remove_entry(ht* table, const char* key, int uid);

ht_entry_item* get_entry_item(ht* table, const char* key, int uid);

void print_entries_in_subtable(ht* table, int uid, size_t bucket_index);

#endif // _HASH_H