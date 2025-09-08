#include <asio/error_code.hpp>
#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "asio/error_code.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

// Define a callback to handle incoming messages
void
on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::cout << "on_message called with hdl: " << hdl.lock().get() << " and message: " << msg->get_payload()
              << std::endl;

    // // check for a special command to instruct the server to stop listening so
    // // it can be cleanly exited.
    // if (msg->get_payload() == "stop-listening")
    // {
    //     s->stop_listening();
    //     return;
    // }

    auto message = msg->get_payload();

    nlohmann::json j = nlohmann::json::parse(message);

    try
    {
        s->send(hdl, msg->get_payload(), msg->get_opcode());
    } catch (websocketpp::exception const& e)
    {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}

void
handler(std::mutex& mtx, server& server, websocketpp::connection_hdl hdl, std::shared_ptr<asio::steady_timer> timer,
        const asio::error_code& error)
{
    // std::cout << "TICK" << std::endl;

    if (!error)
    {
        nlohmann::json j;
        nlohmann::json jin;
        jin["position"] = {
            static_cast<double>(rand() % 100) / 10.0,
            0.0,
            static_cast<double>(rand() % 100) / 10.0,
        };
        j["PlayerState"] = jin;

        std::string                 welcome_message = j.dump();
        // Send a welcome message to the client
        std::lock_guard<std::mutex> lock(mtx);
        server.send(hdl, welcome_message, websocketpp::frame::opcode::text);
        // Set another timer for 1 second in the future
        // asio::steady_timer timer(server.get_io_service(), std::chrono::steady_clock::now() +
        // std::chrono::seconds(1));
        timer->expires_at(timer->expiry() + std::chrono::seconds(1));
        timer->async_wait(bind(&handler, std::ref(mtx), std::ref(server), hdl, timer, std::placeholders::_1));
    } else
    {
        std::cout << "Timer error: " << error.message() << std::endl;
    }
}

int
main()
{
    // Create a server endpoint
    std::mutex mtx;
    server     echo_server;

    try
    {
// Set logging settings
#ifndef NDEBUG
        echo_server.set_access_channels(websocketpp::log::alevel::none);
#endif
        echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        echo_server.set_error_channels(websocketpp::log::elevel::none);
        // Initialize Asio
        echo_server.init_asio();

        echo_server.set_open_handler([&](websocketpp::connection_hdl hdl) {
            std::cout << "Client connected: " << hdl.lock().get() << std::endl;

            std::shared_ptr<asio::steady_timer> timer = std::make_shared<asio::steady_timer>(
                echo_server.get_io_service(), std::chrono::steady_clock::now() + std::chrono::seconds(1));

            timer->async_wait(
                std::bind(&handler, std::ref(mtx), std::ref(echo_server), hdl, timer, std::placeholders::_1));
        });
        // Register our message handler
        echo_server.set_message_handler(bind(&on_message, &echo_server, ::_1, ::_2));

        // Listen on port 9002
        echo_server.listen(9002);

        // Start the server accept loop
        echo_server.start_accept();

        // Start the ASIO io_service run loop
        for (int i = 0; i < std::thread::hardware_concurrency() - 1; ++i)
            std::thread([&]() { echo_server.run(); }).detach();
        echo_server.run();

    } catch (websocketpp::exception const& e)
    {
        std::cout << e.what() << std::endl;
    } catch (...)
    {
        std::cout << "other exception" << std::endl;
    }
}
