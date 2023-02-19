#ifndef __OS_H__
#define __OS_H__
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

static size_t read_all_content(void **buffer, const char *filename)
{
    *buffer = NULL;
    FILE *fd = fopen(filename, "rb");
    if (fd == NULL)
        return 0;
    fseek(fd, 0, SEEK_END);
    size_t len = (size_t)ftell(fd);
    fseek(fd, 0, SEEK_SET);
    *buffer = (void *)calloc(sizeof(char), len + 1);
    fread(*buffer, len, 1, fd);
    fclose(fd);
    ((char *)*buffer)[len] = 0;
    return len;
}

/**
 * @brief Get the size of data cache
 * 
 * @param cpu CPU id to query from
 * @param level The level of data cache. Use -1 for LLC
 * @return The size of cache. -1 if error occur
 * 
 * @note The result is query from "/sys/devices/system/cpu0/cache/" on LINUX,
 *       or by running "sysctl -a" on MacOS (TODO)
 */
static int get_datacache_size(int level)
{
    int last_size = -1;
#ifdef __linux__
    int index = 0;
    do {
        char dir[256];
        char type_file[512];
        char size_file[512];
        char level_file[512];
        snprintf(dir, sizeof(dir), "/sys/devices/system/cpu/cpu0/cache/index%d", index);
        snprintf(type_file, sizeof(type_file), "%s/type", dir);
        snprintf(size_file, sizeof(size_file), "%s/size", dir);
        snprintf(level_file, sizeof(level_file), "%s/level", dir);

        char *type_buffer = NULL;
        char *size_buffer = NULL;
        char *level_buffer = NULL;
        size_t type_slen = read_all_content(&type_buffer, type_file);
        size_t size_slen = read_all_content(&size_buffer, size_file);
        read_all_content(&level_buffer, level_file);
        if (type_buffer == NULL || size_buffer == NULL || level_buffer == NULL)
            break;

        int is_unified = strncmp(type_buffer, "Unified", type_slen) == 0;
        int is_data = strncmp(type_buffer, "Data", type_slen) == 0;
        if (!is_unified && !is_data)
            continue;

        int curr_size = atoi(size_buffer);
        int curr_level = atoi(level_buffer);        
        if (curr_level == level)
            return curr_size;
        last_size = curr_size;
    } while (1);
#elif defined(__APPLE__)
#endif
    int asked_LLC = level == -1;
    return asked_LLC ? last_size : -1;
}

#endif /*__OS_H__*/