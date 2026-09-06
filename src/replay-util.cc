#include "capture.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <cinttypes>

namespace {
    constexpr uint8_t BINARY_VERSION = 1;
}

void generate_capture(const char *jsonl_path, const char *out_path)
{
    std::ofstream to_write(out_path, std::ios::binary);
    std::ifstream file(jsonl_path);
    std::string buffer;
    uint64_t t = 1000;

    if (!file) {
        fprintf(stderr, "fatal: cannot open %s\n", jsonl_path);
        return;
    }

    if (!to_write) {
        fprintf(stderr, "fatal: cannot open %s\n", out_path);
        return;
    }

    to_write.write(reinterpret_cast<const char *>(&BINARY_VERSION), sizeof(BINARY_VERSION));

    while (std::getline(file, buffer)) {
        uint16_t len = buffer.size();

        to_write.write(reinterpret_cast<const char *>(&t), sizeof(t));
        to_write.write(reinterpret_cast<const char *>(&len), sizeof(len));
        to_write.write(buffer.data(), len);

        t += 1000;
    }
}

void read_capture(const char *path)
{
    std::ifstream to_read(path, std::ios::binary);
    std::vector<uint64_t> samples;
    std::vector<char> payload;
    uint64_t recv_time;
    uint8_t version;
    uint16_t len;
    size_t n;

    if (!to_read) {
        fprintf(stderr, "fatal: cannot open %s\n", path);
        return;
    }

    to_read.read(reinterpret_cast<char *>(&version), sizeof(version));

    if (!to_read) {
        fprintf(stderr, "fatal: empty or truncated capture\n");
        return;
    }

    if (version != BINARY_VERSION) {
        fprintf(stderr, "fatal: wrong version, expected %d but read %d\n", BINARY_VERSION, version);
        return;
    }

    while (to_read.read(reinterpret_cast<char *>(&recv_time), sizeof(recv_time))) {
        if (!to_read.read(reinterpret_cast<char *>(&len), sizeof(len)))
            break;

        payload.resize(len);

        if (!to_read.read(payload.data(), len))
            break;

        auto a = std::chrono::steady_clock::now();
        /*
         * ADD HERE THE PROCESS FUNCTION
         */
        auto b = std::chrono::steady_clock::now();

        samples.push_back((b - a).count());
    }

    if (samples.empty())
        return;

    std::sort(samples.begin(), samples.end());

    n = samples.size();
    printf("n=%zu p50=%" PRIu64 " p99=%" PRIu64 " p99.9=%" PRIu64 "\n",
           n,
           samples[n / 2],
           samples[n * 99 / 100],
           samples[n * 999 / 1000]);
}
