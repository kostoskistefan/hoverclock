#ifndef VAULT_H
#define VAULT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef struct vault_s vault_s;

vault_s *vault_create(const char *vault_name);
void vault_destroy(vault_s *const vault);

char *vault_get_file_path(vault_s *vault);
uint8_t vault_insert(vault_s *const vault, const char *key, const char *value);
char *vault_find(vault_s *const vault, const char *key);
char *vault_find_or_default(vault_s *const vault, const char *key, char *default_value);
uint8_t vault_update(vault_s *const vault, const char *key, const char *value);

uint8_t vault_save(vault_s *const vault);
uint8_t vault_load(vault_s *const vault);

void vault_dump(vault_s *const vault);

// Platform specific functions
uint8_t vault_file_exists(vault_s *const vault);
char *vault_get_path_from_vault_name(const char *vault_name);
void vault_create_required_directories(const char *path);

#ifdef __cplusplus
}
#endif

#endif // VAULT_H
