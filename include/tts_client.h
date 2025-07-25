#ifndef TTS_CLIENT_H
#define TTS_CLIENT_H

#include <string>

class TTSClient {
public:
    TTSClient(std::string appid, std::string api_key, std::string api_secret);
    bool synthesize(const std::string& text, const std::string& output_filename);

private:
    std::string _appid;
    std::string _api_key;
    std::string _api_secret;
};

#endif // TTS_CLIENT_H
