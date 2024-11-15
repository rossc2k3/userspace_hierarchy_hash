#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

struct hash_table
{
    int uid;
    int value;
    struct hash_table* next;
};

int main()
{
    printf("Testbench for userspace hierarchical hash implementation.\n");
    printf("Please choose one of the following commands:\n");
    printf("1. \"add\": Add a new value to the hash table, provided a theoretical uid and a value.\n");
    printf("   Will be hashed to a bucket based on the addr and the uid.\n");
    printf("2. \"remove\": Remove a value from the hash table, given a theoretical uid and a value.\n");
    printf("   If this was the only value in a bucket, the bucket should be destroyed.\n");
    printf("3. \"print\": Print the contents of a bucket (i.e its sub buckets and their contents).\n");
    printf("4. \"exit\": Exit the program.\n");
    char* command = (char*)malloc(10);

    #define BUCKETS_AMNT 8192  // mirrors the amount of buckets in the futex hash table

    ht* table = ht_create(BUCKETS_AMNT);
    if(table == NULL)
    {
        perror("Error: Memory allocation for main table failed\n");
        free(table);
        return 1;
    }

    //as a testbench, we make the presumption that the user will enter a valid command

    do
    {
        printf("Enter command: ");
        scanf("%s", command);
        printf("Command entered: %s\n", command);

        if(strcmp(command, "add") == 0)
        {
            char* key = (char*)malloc(64);
            char* value = (char*)malloc(64);

            if(key == NULL || value == NULL)
            {
                perror("Error: Memory allocation failed\n");
                free(key);
                free(value);
                return 1;
            }

            int uid;

            printf("Enter value: ");
            scanf("%s", value);

            printf("Enter uid: ");
            scanf("%d", &uid);

            printf("uid is %d\n", uid);

            ht_add_entry(table, key, value, uid);

            ht_entry_item* subentry = get_entry_item(table, key, uid);

            printf("Entry added successfully\n");
            printf("Key: %p\n", (void*)subentry->key);
            printf("Value: %s\n", (char*)subentry->value);
        }
        else if(strcmp(command, "remove") == 0)
        {
            //doesn't work yet

            char* key = (char*)malloc(64);

            if(key == NULL)
            {
                perror("Error: Memory allocation failed\n");
                free(key);
                return 1;
            }

            int uid;

            printf("Enter key: ");
            scanf("%s", key);

            printf("Enter uid: ");
            scanf("%d", &uid);

            int remove_code = ht_remove_entry(table, key, uid);

            printf("remove code: %d\n", remove_code);

            if(remove_code == -1)
            {
                perror("Error: Entry not found\n");
                free(key);
                continue;
            }

            printf("Entry removed successfully\n");

        }
        else if(strcmp(command, "print") == 0)
        {
            int uid;
            size_t bucket_index;

            printf("Enter uid: ");
            scanf("%d", &uid);

            
            printf("Enter bucket index: ");
            scanf("%lu", &bucket_index);

            print_entries_in_subtable(table, uid, bucket_index);
        }   
        else
        {
            printf("Invalid command\n");
        }


    } while (strcmp (command, "exit") != 0);

    return 0;
}