    #include "capture.h"
    #include "util.h"
    #include "binance-parser.h"

    #include <algorithm>
    #include <chrono>
    #include <cinttypes>
    #include <cstring>
    #include <fstream>
    #include <numeric>
    #include <string>
    #include <vector>
    #include <array>

    namespace {
    constexpr uint8_t BINARY_VERSION = 1;
    }

    /*
    * TODO: @ingest.
    * Once the ingest can write its own capture files this function SHOULD be removed.
    */
    void generate_capture(const char *jsonl_path, const char *out_path) {
        std::ofstream to_write(out_path, std::ios::binary);
        std::ifstream file(jsonl_path);
        std::string buffer;
        /* fake timestamp */
        uint64_t t = 1000;

        if (!file)
            die("cannot open %s\n", jsonl_path);

        if (!to_write)
            die("cannot open %s\n", out_path);

        /*
        * Write hardcoded version to avoid having regresions between stored binaries.
        */
        to_write.write(reinterpret_cast<const char *>(&BINARY_VERSION), sizeof(BINARY_VERSION));

        /*
        * For each line write:
        * [8:TIME][2:LEN][LEN:PAYLOAD]
        */
        while (std::getline(file, buffer)) {
            uint16_t len = buffer.size();

            to_write.write(reinterpret_cast<const char *>(&t), sizeof(t));
            to_write.write(reinterpret_cast<const char *>(&len), sizeof(len));
            to_write.write(buffer.data(), len);

            t += 1000;
        }
    }

    void read_capture(const char *path) {
        std::ifstream to_read(path, std::ios::binary);
        std::vector<uint64_t> samples;
        std::vector<char> payload;
        uint64_t recv_time;
        uint8_t version;
        uint16_t len;
        size_t n;

        std::array<event_atom, 256> events;

        if (!to_read) {
            die("cannot open %s\n", path);
        }

        to_read.read(reinterpret_cast<char *>(&version), sizeof(version));

        if (!to_read)
            die("empty or truncated capture\n");

        if (version != BINARY_VERSION)
            die("wrong version, expected %d but read %d\n", BINARY_VERSION, version);

        /*
        * Read each line as:
        * [8:TIME][2:LEN][LEN:PAYLOAD]
        *
        * Reads until EOF or a malformed sample.
        */
        while (to_read.read(reinterpret_cast<char *>(&recv_time), sizeof(recv_time))) {
            if (!to_read.read(reinterpret_cast<char *>(&len), sizeof(len)))
                break;

            payload.resize(len);

            if (!to_read.read(payload.data(), len))
                break;

            auto a = std::chrono::steady_clock::now();

            const ParseResult result =
                parse_binance_depth(
                    std::string_view(payload.data(), payload.size()),
                    recv_time,
                    events);

            auto b = std::chrono::steady_clock::now();

            if (result.error != ParseError::NONE) {
                die("failed to parse Binance depth message\n");
            }
            
            /* Test
            for (std::size_t i = 0; i < result.count; ++i) {
                    const event_atom &event = events[i];

                    printf(
                        "event %zu: side=%s price=%" PRIu64
                        " quantity=%" PRIu64
                        " update_id=%" PRIu64
                        " exchange_time=%" PRIu64 "\n",
                        i,
                        event.side == Side::BID ? "BID" : "ASK",
                        event.price,
                        event.quantity,
                        event.update_id,
                        event.exchange_time);
            }
            */

            samples.push_back((b - a).count());
        }

        if (samples.empty())
            return;

        std::sort(samples.begin(), samples.end());

        n = samples.size();
        printf("n=%zu p50=%" PRIu64 " p99=%" PRIu64 " p99.9=%" PRIu64 "\n", n, samples[n / 2],
            samples[n * 99 / 100], samples[n * 999 / 1000]);

        uint64_t total = std::accumulate(samples.begin(), samples.end(), uint64_t{0});

        printf("throughput=%.0f msg/s\n", 1e9 * static_cast<double>(n) / static_cast<double>(total));
    }
