# Roadmap

## First

- `event.h`: shared event structure. Blocks ingest and order book.
- `capture.h`: capture file format. Blocks ingest and measurement.

## Then, in parallel

**Ingest**
TCP socket, TLS with OpenSSL, WebSocket handshake, frame reading. Snapshot sync
and gap detection. Writes raw messages to a capture file.

- RFC 6455, sections 4.1 and 5.2: https://www.rfc-editor.org/rfc/rfc6455
- https://developers.binance.com/docs/binance-spot-api-docs/web-socket-streams

**Order book**
Two price-ordered containers. Zero quantity removes the level, anything else
replaces it, unknown price creates it. Tested against `testdata/`.

- https://gist.github.com/halfelf/db1ae032dc34278968f8bf31ee999a25
- https://docs.rs/crate/lobster/latest/source/quantcup/engine.c

**Transport**
SPSC queue. Pick the reference machine and document how to configure it.

- https://github.com/rigtorp/SPSCQueue
- https://github.com/rigtorp/awesome-lockfree

**Measurement**
Generator from `testdata/sample.jsonl` to a capture file. Replayer. Latency
histograms and percentiles.

- https://github.com/HdrHistogram/HdrHistogram_c
- Gil Tene, "How NOT to Measure Latency"

## Milestone

Run the pipeline end to end, however slow, and record the number.

## Rule

No optimisation lands without a measurement backing it.
