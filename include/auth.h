#ifndef AUTH_H
#define AUTH_H

#include <string>

// 为星火大模型或TTS服务生成完整的、带鉴权的URL
std::string generate_auth_url(const std::string& host, const std::string& path, const std::string& api_key, const std::string& api_secret);

// 将 base64_encode 的声明放在头文件，使其可以被其他文件调用
std::string base64_encode(const unsigned char* input, int length);

// **核心修正：添加 base64_decode 的声明**
std::string base64_decode(const std::string& input);

#endif // AUTH_H
