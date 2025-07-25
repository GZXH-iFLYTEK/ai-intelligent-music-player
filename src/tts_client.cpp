#include "tts_client.h"
#include "auth.h"
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <future>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>

using json = nlohmann::json;
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

std::string base64_decode(const std::string& input);

context_ptr on_tls_init_tts_final(websocketpp::connection_hdl) {
    context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12_client);
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
        ctx->set_verify_mode(boost::asio::ssl::verify_peer);
        ctx->load_verify_file("/etc/ssl/certs/ca-certificates.crt");
    } catch (std::exception const & e) {
        std::cout << "Error in TTS TLS init: " << e.what() << std::endl;
    }
    return ctx;
}

TTSClient::TTSClient(std::string appid, std::string api_key, std::string api_secret)
    : _appid(std::move(appid)), _api_key(std::move(api_key)), _api_secret(std::move(api_secret)) {}

bool TTSClient::synthesize(const std::string& text, const std::string& output_filename) {
    client c;
    std::ofstream audio_file(output_filename, std::ios::binary);
    if (!audio_file.is_open()) {
        std::cerr << "Error: Cannot open output file " << output_filename << std::endl;
        return false;
    }

    std::promise<bool> promise;
    auto future = promise.get_future();
    std::atomic<bool> promise_fulfilled(false);

    c.set_access_channels(websocketpp::log::alevel::none);
    c.set_error_channels(websocketpp::log::elevel::all);
    c.init_asio();
    c.set_tls_init_handler(&on_tls_init_tts_final);

    c.set_message_handler([&](websocketpp::connection_hdl, client::message_ptr msg) {
        // TTS的成功和失败消息都是JSON格式，但结构不同
        try {
            json response = json::parse(msg->get_payload());
            // 检查失败情况，错误码在顶层
            if (response.contains("code") && response["code"] != 0) {
                std::cerr << "TTS Server Error: " << response["message"].get<std::string>() 
                          << " (code: " << response["code"].get<int>() << ", sid: " << response["sid"].get<std::string>() << ")" << std::endl;
                if (!promise_fulfilled.exchange(true)) { promise.set_value(false); }
                return;
            }
            
            // 检查成功情况
            if (response.contains("data") && response["data"].contains("audio")) {
                std::string audio_b64 = response["data"]["audio"].get<std::string>();
                std::string decoded_audio = base64_decode(audio_b64);
                audio_file.write(decoded_audio.c_str(), decoded_audio.length());
            }

            if (response.contains("data") && response["data"]["status"] == 2) {
                if (!promise_fulfilled.exchange(true)) { promise.set_value(true); }
            }
        } catch(const json::exception& e) {
            std::cerr << "TTS JSON Error: " << e.what() << "\nRaw message: " << msg->get_payload() << std::endl;
            if (!promise_fulfilled.exchange(true)) { promise.set_value(false); }
        }
    });
    
    c.set_fail_handler([&](websocketpp::connection_hdl hdl) { 
        if (!promise_fulfilled.exchange(true)) { promise.set_value(false); }
    });
    c.set_close_handler([&](websocketpp::connection_hdl hdl) {
        if (!promise_fulfilled.exchange(true)) { promise.set_value(false); }
    });

    c.set_open_handler([&](websocketpp::connection_hdl hdl) {
        // **核心修改：构建TTS专用的正确JSON结构**
        json request;
        request["common"]["app_id"] = _appid;
        
        request["business"]["aue"] = "lame";     // **lame表示MP3格式**
        request["business"]["sfl"] = 1;          // **必须设置为1来开启lame的流式返回**
        request["business"]["vcn"] = "xiaoyan";
        request["business"]["tte"] = "UTF8";
        request["business"]["speed"] = 50;
        
        std::string text_b64 = base64_encode(reinterpret_cast<const unsigned char*>(text.c_str()), text.length());
        request["data"]["text"] = text_b64;
        request["data"]["status"] = 2; // 2表示一次性发送所有文本

        c.send(hdl, request.dump(), websocketpp::frame::opcode::text);
    });

    std::string url = generate_auth_url("tts-api.xfyun.cn", "/v2/tts", _api_key, _api_secret);
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(url, ec);
    c.connect(con);
    
    std::thread t([&](){ c.run(); });

    if (future.wait_for(std::chrono::seconds(30)) == std::future_status::timeout) {
        std::cerr << "TTS request timed out." << std::endl;
        c.stop();
        t.join();
        return false;
    }

    bool result = future.get();
    if (con && con->get_state() == websocketpp::session::state::open) {
        c.close(con, websocketpp::close::status::normal, "");
    }
    t.join();
    
    return result;
}
