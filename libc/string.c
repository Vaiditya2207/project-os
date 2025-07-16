/* Standard Library Functions */

#include "../kernel/kernel.h"

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

void *memset(void *ptr, int value, size_t size)
{
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++)
    {
        p[i] = (uint8_t)value;
    }
    return ptr;
}

void *memcpy(void *dest, const void *src, size_t size)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < size; i++)
    {
        d[i] = s[i];
    }
    return dest;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

// Add a helper function for command parsing
int strncmp(const char *str1, const char *str2, size_t n)
{
    while (n > 0 && *str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
        n--;
    }
    return (n == 0) ? 0 : (*(unsigned char *)str1 - *(unsigned char *)str2);
}
