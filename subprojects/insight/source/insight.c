#include "../include/insight.h"
#include <stdio.h>
#include <string.h>

char project_name[INSIGHT_PROJECT_NAME_MAXIMUM_LENGTH] = "insight";

void insight_set_project_name(const char *name)
{
    strcpy(project_name, name);
}

void _insight_info(const char *message, const char *file_name, unsigned int line_number)
{
    fprintf(stdout, "[%s::info] %s:%d: %s\n", project_name, file_name, line_number, message);
}

void _insight_warning(const char *message, const char *file_name, unsigned int line_number)
{
    fprintf(stderr, "[%s::warning] %s:%d: %s\n", project_name, file_name, line_number, message);
}

void _insight_error(const char *message, const char *file_name, unsigned int line_number)
{
    fprintf(stderr, "[%s::error] %s:%d: %s\n", project_name, file_name, line_number, message);
}
