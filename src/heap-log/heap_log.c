#include "esp_heap_caps.h"

#define TAG "HEAP"

struct ByteInfo
{
    double size;
    const char *unit;
};

struct ByteInfo convert_size(size_t in_size)
{
    struct ByteInfo result;

    const char *units[] = {"B", "kB", "mB"};
    double size = (double)in_size;
    int idx = 0;

    while (size >= 1000.0 && idx < 2)
    {
        size /= 1000.0;
        idx++;
    }

    result.size = size;
    result.unit = units[idx];

    return result;
}

void print_byte_info(const char *label, struct ByteInfo byte_info)
{
    printf("%s: %.2f %s\n", label, byte_info.size, byte_info.unit);
}

void log_caps(uint32_t caps, const char *caps_label)
{
    multi_heap_info_t info;
    heap_caps_get_info(&info, caps);
    printf("%s\n", caps_label);

    print_byte_info("FREE", convert_size(info.total_free_bytes));
    print_byte_info("ALOC", convert_size(info.total_allocated_bytes));
    print_byte_info("BLCK", convert_size(info.largest_free_block));
    // print_byte_info("M", convert_size(info.minimum_free_bytes));
    printf("\n");
}

void log_heap()
{
    uint32_t heap_types[] = {
        MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL,
        MALLOC_CAP_SPIRAM,
        // MALLOC_CAP_8BIT
    };

    const char *labels[] = {
        "INTERN",
        "SPIRAM",
        // "MALLOC_CAP_8BIT"
    };

    for (size_t i = 0; i < sizeof(heap_types) / sizeof(heap_types[0]); i++)
    {
        log_caps(heap_types[i], labels[i]);
    }
}
