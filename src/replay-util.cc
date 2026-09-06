#include "capture.h"

#include <cstdint>
#include <cstdio>

namespace {
    constexpr uint8_t BINARY_VERSION = 1;
}

void generate_capture(const char* jsonl_path, const char* out_path)
{
    FILE *file, *to_write;

    file = fopen(jsonl_path, "r");
    if (!file) {
        fprintf(stderr, "fatal: cannot open %s\n", jsonl_path);
        return;
    }

    to_write = fopen(out_path, "wb");
    if (!to_write) {
        fprintf(stderr, "fatal: cannot open %s\n", out_path);
        goto clean_file;
    }

    fwrite(&BINARY_VERSION, sizeof(BINARY_VERSION), 1, to_write);

    fclose(to_write);
clean_file:
    fclose(file);
}
