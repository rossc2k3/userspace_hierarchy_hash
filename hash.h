//this file contains code from the ht repository by Ben Hoyt, which is licensed under the MIT license

#ifndef _HASH_H
#define _HASH_H

#include <stdbool.h>
#include <stddef.h>

//ht_entry_item: contains the actual key-value pair in the subtable
typedef struct {
    const char* key;
    void* value;
} ht_entry_item;

//ht_subentry_list: the uid-specific list which contains the key-value pairs
typedef struct {
    ht_entry_item* items;
    size_t count;
    size_t capacity;
} ht_subentry_list;

//ht_subentry: the actual subentry in the subtable
typedef struct {
    ht_subentry_list* entries;
} ht_subentry;

//ht_subtable: the buckets which uids hash to
typedef struct {
    ht_subentry* entries;  // hash slots
    size_t capacity;       // size of _entries array
    size_t length;         // number of items in hash table
} ht_subtable;

//ht_entry: the actual hash table entry
typedef struct {
    ht_subtable* subtable;
} ht_entry;

//ht: the hash table
typedef struct {
    ht_entry* entries;  // hash slots
    size_t capacity;    // size of _entries array
    size_t length;      // number of items in hash table
} ht;

// Create hash table and return pointer to it, or NULL if out of memory.
ht* ht_create(size_t capacity);

// based on a key's addr and if it's shared or not, hash it to a bucket index
size_t get_bucket_index(void* addr, bool shared);

// create a subtable and return a pointer to it, or NULL if out of memory

ht_subtable* ht_subtable_create(size_t capacity);

// add a value to the hash table based on a key's addr and uid

void ht_add_entry(ht* table, const char* key, void* value, int uid);

// remove a value from the hash table based on a key's addr and uid

int ht_remove_entry(ht* table, const char* key, int uid);

// get an entry item from the hash table based on a key's addr and uid

ht_entry_item* get_entry_item(ht* table, const char* key, int uid);

// print entries at a specific bucket index in the hash table

void print_entries_in_subtable(ht* table, int uid, size_t bucket_index);

#define INIT_SUBLIST_SIZE 32 //32 for now, we will test
#define BUCKETS_AMNT 8192  // mirrors the amount of buckets in the futex hash table
#define SHARED_BUCKET_INDEX (BUCKETS_AMNT - 1) // the index of the shared bucket

#endif // _HASH_H