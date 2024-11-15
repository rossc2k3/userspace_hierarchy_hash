//this file contains code from the ht repository by Ben Hoyt, which is licensed under the MIT license

#include "hash.h"
#include "jhash.c"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define BUCKETS_AMNT 8192  // mirrors the amount of buckets in the futex hash table
#define SHARED_BUCKET_INDEX (BUCKETS_AMNT - 1) // the index of the shared bucket

size_t get_bucket_index(void* addr, bool shared)
{
    if (shared)
    {
        return SHARED_BUCKET_INDEX;
    }
    uint32_t hash = jhash(&addr, sizeof(addr), 0);
    return hash % (BUCKETS_AMNT - 2); // the shared bucket is the last bucket
}

ht* ht_create(size_t capacity) 
{
    // Allocate space for hash table struct.
    ht* table = malloc(sizeof(ht));
    if (table == NULL) {
        return NULL;
    }
    table->length = 0;
    table->capacity = capacity;

    // Allocate (zero'd) space for entry buckets.
    table->entries = calloc(table->capacity, sizeof(ht_entry));
    if (table->entries == NULL) {
        perror("Error: Could not allocate memory\n");
        free(table); // error, free table before we return!
        return NULL;
    }

    //allocate subtables for each bucket

    for(size_t i = 0; i < table->capacity - 1; i++)
    {
        table->entries[i].subtable = ht_subtable_create(32); //we don't need as much space, we're just hashing users to their locations in the table
    }

    return table;

}

ht_subtable* ht_subtable_create(size_t capacity)
{
    ht_subtable* subtable = malloc(sizeof(ht_subtable));
    if (subtable == NULL)
    {
        free(subtable);
        return NULL;
    }
    subtable->length = 0;
    subtable->capacity = capacity;
    subtable->entries = calloc(subtable->capacity, sizeof(ht_subentry));
    if (subtable->entries == NULL)
    {
        free(subtable->entries);
        free(subtable);
        return NULL;
    }
    return subtable;
}

void ht_add_entry(ht* table, const char* key, void* value, int uid)
{


    size_t bucket_index = get_bucket_index((void*)key, 0);

    printf("bucket_index: %lu\n", bucket_index);

    ht_entry* entry = &table->entries[bucket_index];


    ht_subtable* subtable = entry->subtable;
    size_t sub_bucket_index = uid % subtable->capacity;

    printf("sub_bucket_index: %lu\n", sub_bucket_index);

    ht_subentry* subtable_entry = &subtable->entries[sub_bucket_index];

    if(subtable_entry->entries == NULL)
    {
        subtable_entry->entries = malloc(sizeof(ht_subentry_list));
        if(subtable_entry->entries == NULL)
        {
            free(subtable_entry->entries);
            return; // deal with alloc failure later (TODO)
        }

        subtable_entry->entries->count = 0;
        #define INIT_SUBLIST_SIZE 32 //32 for now, we will test
        subtable_entry->entries->capacity = INIT_SUBLIST_SIZE;
        subtable_entry->entries->items = malloc(INIT_SUBLIST_SIZE * sizeof(ht_entry_item));

        if(subtable_entry->entries->items == NULL)
        {
            free(subtable_entry->entries);
            return; // deal with alloc failure later (TODO)
        }
    }

    ht_subentry_list* entry_list = subtable_entry->entries;

    if(entry_list->count >= entry_list->capacity)
    {
        //realloc
        entry_list->capacity *= 2;
        ht_entry_item* new_items = realloc(entry_list->items, entry_list->capacity * sizeof(ht_entry_item));
        if(new_items == NULL)
        {
            free(entry_list->items);
            return; // deal with alloc failure later (TODO)
        }
        entry_list->items = new_items;
    }

    entry_list->items[entry_list->count].key = key;
    entry_list->items[entry_list->count].value = value;
    entry_list->count++;

}

ht_entry_item* get_entry_item(ht* table, const char* key, int uid)
{
    size_t bucket_index = get_bucket_index((void*)key, 0);

    ht_entry* entry = &table->entries[bucket_index];

    if (entry->subtable == NULL)
    {
        perror("Error: Could't find subtable\n");
        free(entry->subtable);
        return NULL;
    }

    ht_subtable* subtable = entry->subtable;
    size_t sub_bucket_index = uid % subtable->capacity;
    ht_subentry* subtable_entry = &subtable->entries[sub_bucket_index];

    if(subtable_entry->entries == NULL)
    {
        perror("Error: Couldn't find subtable entries\n");
        free(subtable_entry->entries);
        return NULL;
    }

    ht_subentry_list* entry_list = subtable_entry->entries;

    for(size_t i = 0; i < entry_list->count; i++)
    {
        if(strcmp(key, entry_list->items[i].key) == 0)
        {
            return &entry_list->items[i];
        }
    }
    perror("Error: No such entry\n");
    return NULL;
}

int ht_remove_entry(ht* table, const char* key, int uid)
{
    ht_entry_item* to_remove = get_entry_item(table, key, uid);

    if(to_remove == NULL)
    {
        perror("Error: Couldn't find entry to remove\n");
        return -1;
    }

    size_t bucket_index = get_bucket_index((void*)key, 0);
    ht_entry* entry = &table->entries[bucket_index];
    ht_subtable* subtable = entry->subtable;
    size_t sub_bucket_index = uid % subtable->capacity;
    ht_subentry* subtable_entry = &subtable->entries[sub_bucket_index];
    ht_subentry_list* entry_list = subtable_entry->entries;

    size_t i;
    for (i = 0; i < entry_list->count; i++)
    {
        if (entry_list->items[i].key == to_remove->key)
        {
            break;
        }
    }

    for (size_t j = i; j < entry_list->count - 1; j++)
    {
        entry_list->items[j] = entry_list->items[j + 1];
    }
    entry_list->count--;

    //if array is too large, shrink it

    if(entry_list->count < entry_list->capacity / 4 && entry_list->capacity > INIT_SUBLIST_SIZE)
    {
        size_t new_capacity = entry_list->capacity / 2;
        if(new_capacity < INIT_SUBLIST_SIZE)
        {
            new_capacity = INIT_SUBLIST_SIZE;
        }
        ht_entry_item* new_items = realloc(entry_list->items, new_capacity * sizeof(ht_entry_item));
        if(new_items == NULL)
        {
            perror("Error: Could not reallocate memory\n");
            return -1;
        }
        entry_list->items = new_items;
        entry_list->capacity = new_capacity;
    }
}

void print_entries_in_subtable(ht* table, int uid, size_t bucket_index)
{
    ht_entry* entry = &table->entries[bucket_index];

    if (entry->subtable == NULL)
    {
        perror("Error: Couldn't find subtable\n");
        free(entry->subtable);
        return;
    }

    ht_subtable* subtable = entry->subtable;

    if(subtable->entries == NULL)
    {
        perror("Error: Couldn't find subtable entries\n");
        free(subtable->entries);
        return;
    }

    size_t sub_bucket_index = uid % subtable->capacity;
    ht_subentry* subtable_entry = &subtable->entries[sub_bucket_index];
    if(subtable_entry->entries == NULL)
    {
        perror("Error: Couldn't find subtable entries\n");
        free(subtable_entry->entries);
        return;
    }

    ht_subentry_list* entry_list = subtable_entry->entries;

    if(entry_list->count == 0)
    {
        perror("Error: No entries in subtable\n");
        return;
    }

    for(size_t i = 0; i < entry_list->count; i++)
    {
        printf("Key: %p\n", (void*)entry_list->items[i].key);
        printf("Value: %s\n", (char*)entry_list->items[i].value);
    }
}

/*void ht_remove_entry(ht* table, const char* key, int uid)
{
    size_t bucket_index = get_bucket_index((void*)key, 0);

    ht_entry* entry = &table->entries[bucket_index];
    if(entry->subtable == NULL)
    {
        perror("Error: No such entry\n");
        return;
    }
    ht_subtable* subtable = entry->subtable;
    size_t sub_bucket_index = uid % subtable->capacity;
    ht_subentry* subtable_entry = &subtable->entries[sub_bucket_index];

    if(subtable_entry->entries == NULL)
    {
        perror("Error: No such entry\n");
        return;
    }

}*/


/*void ht_destroy(ht* table) 
{
    // First free allocated keys.
    for (size_t i = 0; i < table->capacity; i++) {
        free((void*)table->entries[i].key);
    }

    // Then free entries array and table itself.
    free(table->entries);
    free(table);
}*/

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_key(const char* key) 
{
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) 
    {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

/*void* ht_get(ht* table, const char* key) 
{
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    // Loop till we find an empty entry.
    while (table->entries[index].key != NULL) 
    {
        if (strcmp(key, table->entries[index].key) == 0) 
        {
            // Found key, return value.
            return table->entries[index].value;
        }
        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= table->capacity) 
        {
            // At end of entries array, wrap around.
            index = 0;
        }
    }
    return NULL;
}*/

// Internal function to set an entry (without expanding table).
/*static const char* ht_set_entry(ht_entry* entries, size_t capacity,
        const char* key, void* value, size_t* plength) 
{
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    // Loop till we find an empty entry.
    while (entries[index].key != NULL) 
    {
        if (strcmp(key, entries[index].key) == 0) 
        {
            // Found key (it already exists), update value.
            entries[index].value = value;
            return entries[index].key;
        }
        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= capacity) 
        {
            // At end of entries array, wrap around.
            index = 0;
        }
    }

    // Didn't find key, allocate+copy if needed, then insert it.
    if (plength != NULL) 
    {
        key = strdup(key);
        if (key == NULL) 
        {
            return NULL;
        }
        (*plength)++;
    }
    entries[index].key = (char*)key;
    entries[index].value = value;
    return key;
}*/

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
/*static bool ht_expand(ht* table) 
{
    // Allocate new entries array.
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) 
    {
        return false;  // overflow (capacity would be too big)
    }
    ht_entry* new_entries = calloc(new_capacity, sizeof(ht_entry));
    if (new_entries == NULL) 
    {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < table->capacity; i++) 
    {
        ht_entry entry = table->entries[i];
        if (entry.key != NULL) 
        {
            ht_set_entry(new_entries, new_capacity, entry.key,
                         entry.value, NULL);
        }
    }

    // Free old entries array and update this table's details.
    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}*/

/*const char* ht_set(ht* table, const char* key, void* value) 
{
    assert(value != NULL);
    if (value == NULL) 
    {
        return NULL;
    }

    // If length will exceed half of current capacity, expand it.
    if (table->length >= table->capacity / 2) 
    {
        if (!ht_expand(table)) 
        {
            return NULL;
        }
    }

    // Set entry and update length.
    return ht_set_entry(table->entries, table->capacity, key, value,
                        &table->length);
}*/

size_t ht_length(ht* table) 
{
    return table->length;
}

hti ht_iterator(ht* table) 
{
    hti it;
    it._table = table;
    it._index = 0;
    return it;
}

/*bool ht_next(hti* it) 
{
    // Loop till we've hit end of entries array.
    ht* table = it->_table;
    while (it->_index < table->capacity) 
    {
        size_t i = it->_index;
        it->_index++;
        if (table->entries[i].key != NULL) 
        {
            // Found next non-empty item, update iterator key and value.
            ht_entry entry = table->entries[i];
            it->key = entry.key;
            it->value = entry.value;
            return true;
        }
    }
    return false;
}*/