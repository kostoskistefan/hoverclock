#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef insight_info
#warning "insight_info is already defined"
#else
#define insight_info(message) _insight_info(message, __FILE__, __LINE__)
#endif

#ifdef insight_warning
#warning "insight_warning is already defined"
#else
#define insight_warning(message) _insight_warning(message, __FILE__, __LINE__)
#endif

#ifdef insight_error
#warning "insight_error is already defined"
#else
#define insight_error(message) _insight_error(message, __FILE__, __LINE__)
#endif

#define INSIGHT_PROJECT_NAME_MAXIMUM_LENGTH 64

void insight_set_project_name(const char *name);

void _insight_info(const char *message, const char *file_name, unsigned int line_number);
void _insight_warning(const char *message, const char *file_name, unsigned int line_number);
void _insight_error(const char *message, const char *file_name, unsigned int line_number);

#ifdef __cplusplus
}
#endif
