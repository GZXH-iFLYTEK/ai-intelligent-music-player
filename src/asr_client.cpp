#include "asr_client.h"
#include "sparkchain.h"
#include "sc_asr.h"
#include <iostream>
#include <atomic>
#include <unistd.h>
#include <future>

using namespace SparkChain;

static std::promise<std::string> asr_promise;
static std::atomic<bool> asr_finish(false);

class ASRCallbacksImpl : public ASRCallbacks {
    void onResult(ASRResult* result, void* usrTag) override {
        if (result->status() == 2) { // 2表示最终结果
            asr_promise.set_value(result->bestMatchText());
            asr_finish = true;
        }
    }
    void onError(ASRError* error, void* usrTag) override {
        std::string err_msg = "ASR Error: " + std::to_string(error->code()) + " " + error->errMsg();
        asr_promise.set_value(err_msg);
        asr_finish = true;
    }
};

ASRClient::ASRClient(std::string appid, std::string api_key, std::string api_secret) {
    // SDK在主程序中统一初始化，这里不需要重复初始化
}

std::string ASRClient::transcribe(const std::vector<char>& audio_data) {
    ASR asr("zh_cn", "iat", "mandarin"); // 中文普通话识别
    ASRCallbacksImpl callbacks;
    asr.registerCallbacks(&callbacks);

    AudioAttributes attr;
    attr.setSampleRate(16000);
    attr.setEncoding("raw");
    attr.setChannels(1);

    int ret = asr.start(attr);
    if (ret != 0) {
        return "ASR Error: start failed with code " + std::to_string(ret);
    }

    asr_finish = false;
    asr_promise = std::promise<std::string>();
    auto future = asr_promise.get_future();

    const size_t per_frame_size = 1280; // 每次发送1280字节
    size_t sent_len = 0;
    while (sent_len < audio_data.size()) {
        size_t current_size = per_frame_size;
        if (sent_len + per_frame_size > audio_data.size()) {
            current_size = audio_data.size() - sent_len;
        }
        asr.write(audio_data.data() + sent_len, current_size);
        sent_len += current_size;
        usleep(40 * 1000); // 模拟实时音频流，每40ms发送一次
    }

    asr.stop();

    while (!asr_finish) {
        usleep(100 * 1000);
    }

    return future.get();
}
