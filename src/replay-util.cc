#include "capture.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
    constexpr uint8_t BINARY_VERSION = 1;
}

void generate_capture(const char* jsonl_path, const char* out_path)
{
    FILE *file, *to_write;
    char buffer[1024];
    uint64_t t = 1000;

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

    while (fgets(buffer, sizeof(buffer), file)) {
        uint16_t len = strlen(buffer);

        if (len && buffer[len - 1] == '\n')
            len--;

        fwrite(&t, sizeof(t), 1, to_write);
        fwrite(&len, sizeof(len), 1, to_write);
        fwrite(buffer, 1, len, to_write);

        t += 1000;
    }

    fclose(to_write);
clean_file:
    fclose(file);
}
