#ifndef ASR_CLIENT_H
#define ASR_CLIENT_H

#include <string>
#include <vector>

class ASRClient {
public:
    ASRClient(std::string appid, std::string api_key, std::string api_secret);
    std::string transcribe(const std::vector<char>& audio_data);
};

#endif // ASR_CLIENT_H
