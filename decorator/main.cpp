#include <boost/asio.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <map>
#include <string>

using namespace boost::beast;
namespace asio = boost::asio;

std::mutex m;
template < class... Ts >
void
emit(Ts const &...ts)
{
    std::unique_lock< std::mutex > lock(m);
    using expand = int[];
    expand { 0, ((std::cout << ts), 0)... };
}

class decorator_t
{
  private:
    std::map< std::string, std::string > header_values_map_d;

  public:
    explicit decorator_t(std::map< std::string, std::string > headers_map)
    : header_values_map_d(std::move(headers_map)) {};
    // We obtain the header part of message and add the additional tags.
    template < bool isrequest, class Body, class Headers >
    void
    operator()(http::message< isrequest, Body, Headers > &msg) const
    {
        for (const auto &value : header_values_map_d)
            msg.base().insert(value.first, value.second);
        // msg.insert() as outlined in the example for async_handshake_ex seems
        // to not compile.
        emit("decorated message:\n", msg);
    }
};

using stream_type = asio::local::stream_protocol::socket;

struct client
{
    websocket::stream< stream_type > &stream;
    websocket::response_type          response;

    void
    run()
    {
        std::map< std::string, std::string > hdrs;
        hdrs.emplace("Header1", "Value1");
        hdrs.emplace("Header2", "Value2");
        decorator_t decorate(hdrs);
#if BOOST_BEAST_VERSION >= 300
        stream.set_option(websocket::stream_base::decorator(decorate));
        stream.async_handshake(response,
                               "localhost",
                               "/ws/filesettings",
                               [this](error_code ec) { on_accept(ec); });
#else
        stream.async_handshake_ex(response,
                                  "localhost",
                                  "/ws/filesettings",
                                  decorate,
                                  [this](error_code ec) { on_accept(ec); });
#endif
    }

    void
    on_accept(error_code ec)
    {
        emit("client:- response:",
             response,
             "client:- : ",
             ec.message(),
             "\n");
    }
};

int
main()
{
    emit("Beast version: ", BOOST_BEAST_VERSION, "\n");
    using stream_type = asio::local::stream_protocol::socket;
    asio::io_context ioc;

    websocket::stream< stream_type > sender(ioc), receiver(ioc);
    asio::local::connect_pair(sender.next_layer(), receiver.next_layer());

    client c { sender };
    c.run();

    std::thread trcv([&] {
        try
        {
            websocket::request_type req;
            flat_buffer             buf;
            error_code              ec;
            http::read(receiver.next_layer(), buf, req, ec);
            emit("trcv:- received: ", req, "trcv:- :", ec.message(), "\n");
            receiver.accept(req, ec);
            emit("trcv:- accepted: ", ec.message(), "\n");
        }
        catch (std::exception &e)
        {
            emit("receiver: ", e.what(), "\n");
        }
    });

    ioc.run();

    trcv.join();
}