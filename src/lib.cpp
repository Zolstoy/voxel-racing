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

typedef server::message_ptr message_ptr;

typedef std::map<websocketpp::connection_hdl, std::shared_ptr<class client>,
                 std::owner_less<websocketpp::connection_hdl>>
    client_container;

class client
{
    server&                             server_;
    websocketpp::connection_hdl         hdl_;
    std::mutex&                         server_mtx_;
    std::shared_ptr<asio::steady_timer> timer_;

   public:
    client(server& s, websocketpp::connection_hdl hdl, std::mutex& mtx)
        : server_(s)
        , hdl_(hdl)
        , server_mtx_(mtx)
        , timer_(std::make_shared<asio::steady_timer>(s.get_io_service()))
    {}

    void on_tick(const asio::error_code& error)
    {
        if (error)
        {
            std::cout << "Timer error: " << error.message() << std::endl;
            return;
        }

        nlohmann::json j;
        nlohmann::json jin;
        jin["position"] = {
            static_cast<double>(rand() % 100) / 10.0,
            0.0,
            static_cast<double>(rand() % 100) / 10.0,
        };
        j["PlayerState"] = jin;

        std::string welcome_message = j.dump();
        {
            std::lock_guard<std::mutex> lock(server_mtx_);
            server_.send(hdl_, welcome_message, websocketpp::frame::opcode::text);
        }
        next();
    }

    void next()
    {
        timer_->expires_at(timer_->expiry() + std::chrono::seconds(1));
        timer_->async_wait(bind(&client::on_tick, this, std::placeholders::_1));
    }

    void send(std::string const& message)
    {
        std::lock_guard<std::mutex> lock(server_mtx_);
        server_.send(hdl_, message, websocketpp::frame::opcode::text);
    }
};

void
on_message(client_container clients, std::mutex& mtx, websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::cout << "on_message called with hdl: " << hdl.lock().get() << " and message: " << msg->get_payload()
              << std::endl;

    auto message = msg->get_payload();

    nlohmann::json j = nlohmann::json::parse(message);
    try
    {
        // server_->send(hdl, msg->get_payload(), msg->get_opcode());
        if (j.contains("action"))
        {
            std::string action = j["action"];
            if (action == "ping")
            {
                nlohmann::json response;
                response["action"]       = "pong";
                std::string response_str = response.dump();

                std::lock_guard<std::mutex> lock(mtx);
                clients[hdl]->send(response_str);
            }
        }

    } catch (websocketpp::exception const& e)
    {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}

int
start()
{
    std::mutex       mtx;
    server           echo_server;
    client_container clients;

    try
    {
#ifndef NDEBUG
        echo_server.set_access_channels(websocketpp::log::alevel::none);
#endif
        echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        echo_server.set_error_channels(websocketpp::log::elevel::none);

        echo_server.init_asio();

        echo_server.set_open_handler([&](websocketpp::connection_hdl hdl) {
            std::cout << "Client connected: " << hdl.lock().get() << std::endl;

            auto client_p =
                clients.insert(std::make_pair(hdl, std::make_shared<client>(echo_server, hdl, mtx))).first->second;

            client_p->next();
        });

        echo_server.set_close_handler([&](websocketpp::connection_hdl hdl) {
            std::cout << "Client disconnected: " << hdl.lock().get() << std::endl;

            std::lock_guard<std::mutex> lock(mtx);
            clients.erase(hdl);
        });

        echo_server.set_message_handler(
            std::bind(&on_message, std::ref(clients), std::ref(mtx), std::placeholders::_1, std::placeholders::_2));

        echo_server.listen(9002);

        echo_server.start_accept();

        // for (int i = 0; i < std::thread::hardware_concurrency() - 1; ++i)
        // std::thread([&]() { echo_server.run(); }).detach();
        echo_server.run();

    } catch (websocketpp::exception const& e)
    {
        std::cout << e.what() << std::endl;
    } catch (...)
    {
        std::cout << "other exception" << std::endl;
    }
}
