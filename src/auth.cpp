#include "auth.h"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <vector>
#include <memory>

// 将所有 base64 工具函数移到全局范围，以匹配 auth.h 中的声明
std::string base64_encode(const unsigned char* input, int length) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    std::string output(bptr->data, bptr->length);
    BIO_free_all(b64);
    return output;
}

std::string base64_decode(const std::string& input) {
    BIO *bio, *b64;
    std::vector<char> buffer(input.length());
    bio = BIO_new_mem_buf(input.data(), -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int decoded_size = BIO_read(bio, buffer.data(), buffer.size());
    BIO_free_all(bio);
    return std::string(buffer.data(), decoded_size);
}

// 只在当前文件使用的内部帮助函数，可以继续保留在匿名命名空间中
namespace {
    std::string get_gmt_time() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&in_time_t), "%a, %d %b %Y %T GMT");
        return ss.str();
    }

    std::string url_encode(const std::string& value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;
        for (char c : value) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else {
                escaped << '%' << std::uppercase << std::setw(2) << int((unsigned char) c);
            }
        }
        return escaped.str();
    }

    std::vector<unsigned char> hmac_sha256(const std::string& key, const std::string& data) {
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int digest_len;
        HMAC(EVP_sha256(), key.c_str(), key.length(), (unsigned char*)data.c_str(), data.length(), digest, &digest_len);
        return std::vector<unsigned char>(digest, digest + digest_len);
    }
}

std::string generate_auth_url(const std::string& host, const std::string& path, const std::string& api_key, const std::string& api_secret) {
    std::string date = get_gmt_time();

    std::string signature_origin = "host: " + host + "\n";
    signature_origin += "date: " + date + "\n";
    signature_origin += "GET " + path + " HTTP/1.1";
    
    // 之前验证过，我们应该直接使用原始secret，而不是解码它
    auto signature_raw = hmac_sha256(api_secret, signature_origin);
    std::string signature_b64 = base64_encode(signature_raw.data(), signature_raw.size());

    std::string authorization_origin = "api_key=\"" + api_key + "\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"" + signature_b64 + "\"";
    std::string authorization_b64 = base64_encode((const unsigned char*)authorization_origin.c_str(), authorization_origin.length());
    
    std::string encoded_auth = url_encode(authorization_b64);
    std::string encoded_date = url_encode(date);

    std::stringstream url_builder;
    url_builder << "wss://" << host << path
                << "?authorization=" << encoded_auth
                << "&date=" << encoded_date
                << "&host=" << host;
    
    return url_builder.str();
}
