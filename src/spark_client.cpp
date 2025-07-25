#include "spark_client.h"
#include "auth.h"
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include "nlohmann/json.hpp"
#include <iostream>
#include <future>
#include <thread>
#include <memory>

using json = nlohmann::json;
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

// 加强的TLS初始化函数
context_ptr on_tls_init_final(websocketpp::connection_hdl) {
    context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12_client);
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
        
        // **核心修改：明确设置证书验证模式和路径**
        ctx->set_verify_mode(boost::asio::ssl::verify_peer);
        ctx->load_verify_file("/etc/ssl/certs/ca-certificates.crt");

    } catch (std::exception const & e) {
        std::cout << "Error in TLS init: " << e.what() << std::endl;
    }
    return ctx;
}

SparkClient::SparkClient(std::string appid, std::string api_key, std::string api_secret)
    : _appid(std::move(appid)), _api_key(std::move(api_key)), _api_secret(std::move(api_secret)) {}

// ask 函数的其他部分保持不变...
// (为简洁起见，此处省略 ask 函数的其余代码，你只需替换文件顶部和此处的 on_tls_init_final 函数即可)
// 确保在 c.set_tls_init_handler 中使用的是 on_tls_init_final
// ...
std::string SparkClient::ask(const std::string& question) {
    client c;
    std::string final_response;
    std::promise<void> promise;
    auto future = promise.get_future();
    bool error_occured = false;

    c.set_access_channels(websocketpp::log::alevel::none);
    c.set_error_channels(websocketpp::log::elevel::all);
    
    // 使用新的TLS初始化函数
    c.init_asio();
    c.set_tls_init_handler(&on_tls_init_final);

    c.set_message_handler([&](websocketpp::connection_hdl, client::message_ptr msg) {
        try {
            json response = json::parse(msg->get_payload());

            if (response["header"]["code"] != 0) {
                std::string error_msg = response["header"]["message"].get<std::string>();
                int error_code = response["header"]["code"].get<int>();
                final_response = "Error: Spark API Error: " + error_msg + " (code: " + std::to_string(error_code) + ")";
                error_occured = true;
                promise.set_value();
                return;
            }

            if (response.contains("payload") && response["payload"].contains("choices") && 
                response["payload"]["choices"].contains("text") && !response["payload"]["choices"]["text"].empty()) 
            {
                final_response += response["payload"]["choices"]["text"][0]["content"].get<std::string>();
            }

            if (response["header"]["status"] == 2) {
                promise.set_value();
            }
        } catch (const json::exception& e) {
            final_response = "Error: JSON parse error: " + std::string(e.what());
            error_occured = true;
            promise.set_value();
        }
    });

    c.set_fail_handler([&](websocketpp::connection_hdl hdl) {
         final_response = "Error: Connection Failed.";
         error_occured = true;
         promise.set_value();
    });

    c.set_close_handler([&](websocketpp::connection_hdl hdl) {
        if (!error_occured) {
             try { promise.set_value(); } catch (const std::future_error& e) {}
        }
    });
    
    c.set_open_handler([&](websocketpp::connection_hdl hdl) {
        json request;
        request["header"]["app_id"] = _appid;
        request["parameter"]["chat"]["domain"] = "4.0Ultra";
        request["payload"]["message"]["text"] = json::array({
            {{"role", "user"}, {"content", question}}
        });
        c.send(hdl, request.dump(), websocketpp::frame::opcode::text);
    });

    std::string url = generate_auth_url("spark-api.xf-yun.com", "/v4.0/chat", _api_key, _api_secret);
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(url, ec);
    if (ec) return "Error: Could not create connection: " + ec.message();

    c.connect(con);
    
    std::thread t([&](){ c.run(); });
    
    if (future.wait_for(std::chrono::seconds(30)) == std::future_status::timeout) {
        final_response = "Error: Request timed out.";
    }
    
    if (con->get_state() == websocketpp::session::state::open) {
        c.close(con, websocketpp::close::status::normal, "");
    }
    t.join();

    return final_response;
}
