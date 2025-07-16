/* VGA Text Mode Driver */

#include "../kernel.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;
static uint16_t *vga_buffer;

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg)
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

void vga_init(void)
{
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_buffer = (uint16_t *)VGA_MEMORY;

    // Test if VGA memory is accessible
    vga_buffer[0] = vga_entry('T', vga_color);
}

void vga_set_color(uint8_t foreground, uint8_t background)
{
    vga_color = vga_entry_color(foreground, background);
}

void vga_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

void vga_scroll(void)
{
    // Move all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
    {
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }

    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++)
    {
        vga_putentryat(' ', vga_color, x, VGA_HEIGHT - 1);
    }

    vga_row = VGA_HEIGHT - 1;
    vga_column = 0;
}

void vga_putchar(char c)
{
    // Safety check
    if (!vga_buffer)
        return;

    if (c == '\n')
    {
        vga_column = 0;
        if (++vga_row >= VGA_HEIGHT)
        {
            vga_row = VGA_HEIGHT - 1;
            vga_scroll();
        }
        return;
    }

    if (c == '\r')
    {
        vga_column = 0;
        return;
    }

    if (c == '\t')
    {
        vga_column = (vga_column + 8) & ~(8 - 1);
        if (vga_column >= VGA_WIDTH)
        {
            vga_column = 0;
            if (++vga_row >= VGA_HEIGHT)
            {
                vga_row = VGA_HEIGHT - 1;
                vga_scroll();
            }
        }
        return;
    }

    vga_putentryat(c, vga_color, vga_column, vga_row);

    if (++vga_column >= VGA_WIDTH)
    {
        vga_column = 0;
        if (++vga_row >= VGA_HEIGHT)
        {
            vga_row = VGA_HEIGHT - 1;
            vga_scroll();
        }
    }
}

void vga_print(const char *data)
{
    for (size_t i = 0; data[i] != '\0'; i++)
    {
        if (data[i] == '\b')
        {
            // Handle backspace
            if (vga_column > 0)
            {
                vga_column--;
                vga_putentryat(' ', vga_color, vga_column, vga_row);
            }
            else if (vga_row > 0)
            {
                vga_row--;
                vga_column = VGA_WIDTH - 1;
                vga_putentryat(' ', vga_color, vga_column, vga_row);
            }
        }
        else
        {
            vga_putchar(data[i]);
        }
    }
}

void vga_clear(void)
{
    for (size_t y = 0; y < VGA_HEIGHT; y++)
    {
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
}

void vga_print_hex(uint32_t value)
{
    char buffer[11]; // "0x" + 8 hex digits + null terminator
    buffer[0] = '0';
    buffer[1] = 'x';
    
    // Convert to hex string
    const char hex_chars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        buffer[2 + (7 - i)] = hex_chars[(value >> (i * 4)) & 0xF];
    }
    buffer[10] = '\0';
    
    vga_print(buffer);
}
