#ifndef CAPTURE_H
#define CAPTURE_H

#include <cstdint>

/*
 * The structure of the whole capture is stored as:
 * FILE:
 *
 * [1:VERSION]
 * [8:RECV_TIME][2:LEN][LEN:PAYLOAD]
 * [8:RECV_TIME][2:LEN][LEN:PAYLOAD]
 * ...
 * EOF
 *
 * Little endian.
 */

struct capture_header {
    /*
     * Right thing to do? Perhaps... Let's have a hardcoded number to avoid having different
     * binaries.
     */
    uint8_t version;
};

struct record_header {
    /*
     * Time in ns ASAP.
     */
    uint64_t recv_time;
    /*
     * Payload length in bytes. The payload is the raw exchange message,
     * unparsed.
     */
    uint16_t len;
};

#endif
