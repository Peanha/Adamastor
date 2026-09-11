#include "client.h"
#include "util.h"

#include <array>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace {
constexpr std::string_view BINANCE_HOST = "stream.binance.com";
constexpr std::string_view BINANCE_PORT = "9443";
constexpr std::string_view BINANCE_ENDPOINT = "/ws/btcusdt@depth@100ms";
/* Maximum message size seen: 28063 bytes */
constexpr std::size_t MAX_MESSAGE_SIZE = UINT16_MAX;
using MESSAGE_BUFFER = std::array<char, MAX_MESSAGE_SIZE>;

/*
 * Binance does not support UDP or a custom protocol.
 * It works with TCP + WebSockets.
 *
 * While not ideal, being at this early stage justifies losing some performance, looking forward
 * to making it work rather than _perfect_ now.
 *
 * OPTIMIZE:
 * This could get optimized by getting rid of 'boost' and just using pure OpenSSL, dropping the lib
 * generalization overhead, and making it specific.
 */
void connect_binance(MESSAGE_BUFFER &buffer) {
    namespace net = boost::asio;
    namespace ssl = net::ssl;
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    using tcp = net::ip::tcp;

    /* Setup */
    net::io_context ioc;
    ssl::context ctx{ssl::context::tls_client};
    SSL_CTX_set_min_proto_version(ctx.native_handle(), TLS1_2_VERSION);

    /* Demand SSL */
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(ssl::verify_peer);

    /*
     * Bottom [LAYER 1] TCP
     *        [LAYER 2] TLS
     * Up     [LAYER 3] WEBSOCKET
     */
    websocket::stream<ssl::stream<tcp::socket>> ws{ioc, ctx};

    /* DNS + TCP */
    tcp::resolver resolver{ioc};                                         /* Create DNS resolver */
    const auto endpoints = resolver.resolve(BINANCE_HOST, BINANCE_PORT); /* Get IPs */
    net::connect(beast::get_lowest_layer(ws), endpoints);

    if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), BINANCE_HOST.data()))
        die("cannot set SNI");

    ws.next_layer().set_verify_callback(ssl::host_name_verification(std::string{BINANCE_HOST}));
    ws.next_layer().handshake(ssl::stream_base::client);

    /* WebSocket */
    const std::string host = std::string{BINANCE_HOST} + ":" + std::string{BINANCE_PORT};
    ws.handshake(host, BINANCE_ENDPOINT);

#ifdef DEBUG
    size_t max_message_size = 0;
#endif
    for (;;) {
        size_t len = 0;

        do {
            if (len == buffer.size())
                throw std::runtime_error("message too large");
            len += ws.read_some(net::buffer(buffer.data() + len, buffer.size() - len));
        } while (!ws.is_message_done());
#ifdef DEBUG
        if (len > max_message_size) {
            max_message_size = len;
            std::fprintf(stderr, "max message %zu bytes\n", max_message_size);
        }
#endif
        /*
         * TODO: JSON PARSER
         */
    }
}
} // namespace

void start_session_binance() {
    using namespace std::chrono_literals;

    MESSAGE_BUFFER buffer;
    for (;;) {
        const auto start = std::chrono::steady_clock::now();
        try {
            connect_binance(buffer);
        } catch (const std::exception &e) {
            fprintf(stderr, "%s\n", e.what());
        }

        if (std::chrono::steady_clock::now() - start < 10s)
            std::this_thread::sleep_for(2s);
    }
}
