#include "../include/vault.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VAULT_INITIAL_CAPACITY 16

typedef struct vault_entry_s
{
    char *key;
    char *value;
} vault_entry_s;

typedef struct vault_s
{
    uint16_t capacity;
    uint16_t entry_amount;
    char *file_path;
    struct vault_entry_s **entries;
} vault_s;

vault_s *vault_create(const char *vault_name)
{
    vault_s *vault = (vault_s *) malloc(sizeof(vault_s));

    vault->capacity = VAULT_INITIAL_CAPACITY;
    vault->entry_amount = 0;
    vault->entries = (vault_entry_s **) calloc(VAULT_INITIAL_CAPACITY, sizeof(vault_entry_s *));
    vault->file_path = vault_get_path_from_vault_name(vault_name);

    return vault;
}

void vault_destroy(vault_s *const vault)
{
    for (uint16_t i = 0; i < vault->entry_amount; ++i)
    {
        free(vault->entries[i]->key);
        free(vault->entries[i]->value);
        free(vault->entries[i]);
    }

    free(vault->entries);
    free(vault->file_path);
    free(vault);
}

char *vault_get_file_path(vault_s *const vault)
{
    return vault->file_path;
}

uint8_t vault_insert(vault_s *const vault, const char *key, const char *value)
{
    for (uint16_t i = 0; i < vault->entry_amount; ++i)
    {
        if (strcmp(vault->entries[i]->key, key) == 0)
        {
            fprintf(stderr, "vault::error: Failed to insert entry. Key exists.\n");
            return 0;
        }
    }

    if (vault->entry_amount == vault->capacity)
    {
        vault->capacity += VAULT_INITIAL_CAPACITY;
        vault->entries = (vault_entry_s **) realloc(vault->entries, sizeof(vault_entry_s *) * vault->capacity);
    }

    vault_entry_s *entry = (vault_entry_s *) malloc(sizeof(vault_entry_s));
    entry->key = strdup(key);
    entry->value = strdup(value);

    vault->entries[vault->entry_amount] = entry;
    ++vault->entry_amount;

    return 1;
}

char *vault_find(vault_s *const vault, const char *key)
{
    for (uint16_t i = 0; i < vault->entry_amount; ++i)
        if (strcmp(vault->entries[i]->key, key) == 0)
            return vault->entries[i]->value;

    return NULL;
}

char *vault_find_or_default(vault_s *const vault, const char *key, char *default_value)
{
    char *value = vault_find(vault, key);
    return value != NULL ? value : default_value;
}

uint8_t vault_update(vault_s *const vault, const char *key, const char *value)
{
    for (uint16_t i = 0; i < vault->entry_amount; ++i)
    {
        if (strcmp(vault->entries[i]->key, key) == 0)
        {
            uint16_t new_value_length = strlen(value);

            if (new_value_length > strlen(vault->entries[i]->value))
                vault->entries[i]->value = (char *) realloc(vault->entries[i]->value, new_value_length);

            strcpy(vault->entries[i]->value, value);

            return 1;
        }
    }

    fprintf(stderr, "vault::error: Failed to update entry. Key does not exists.\n");
    return 0;
}

uint8_t vault_save(vault_s *const vault)
{
    vault_create_required_directories(vault->file_path);

    FILE *file = fopen(vault->file_path, "w");

    if (file == NULL)
    {
        fprintf(stderr, "vault::error: Failed to open vault file in write mode\n");
        return 0;
    }

    for (uint16_t i = 0; i < vault->entry_amount; ++i)
        fprintf(file, "%s=%s\n", vault->entries[i]->key, vault->entries[i]->value);

    fclose(file);

    return 1;
}

uint8_t vault_load(vault_s *const vault)
{
    FILE *file = fopen(vault->file_path, "r");

    if (file == NULL)
    {
        fprintf(stderr, "vault::error: Failed to open vault file in read mode\n");
        return 0;
    }

    char line[1024];

    while (fgets(line, sizeof(line), file))
    {
        char *key = strtok(line, "=\r\n");

        if (!key)
            continue;

        char *value = strtok(NULL, "=\r\n");

        if (!value)
            vault_insert(vault, key, " ");

        else
            vault_insert(vault, key, value);
    }

    fclose(file);

    return 1;
}

void vault_dump(vault_s *const vault)
{
    for (uint16_t i = 0; i < vault->entry_amount; ++i)
        printf("%s=%s\n", vault->entries[i]->key, vault->entries[i]->value);
}
