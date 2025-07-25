#include "spark_client.h"
#include "tts_client.h"
#include "mic_capture.h"
#include "asr_client.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include "sparkchain.h" // 引入SDK主头文件

// --- 你的API凭证 ---
const std::string APPID     = "694d2cf7";
const std::string APIKEY    = "746e3c4d569b202a789f68c8d6834f44";
const std::string APISECRET = "YmYyZjA0NzBlYTA0YTE4MTAwMzJkZWZi";

// 音乐播放的占位函数
void play_music(const std::string& song_name) {
    // 移除歌曲名前后可能存在的空格
    size_t first = song_name.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        std::cout << "[播放器] 未指定有效的歌曲名。" << std::endl;
        return;
    }
    size_t last = song_name.find_last_not_of(" \t\n\r");
    std::string clean_song_name = song_name.substr(first, (last - first + 1));

    std::cout << "[播放器] 正在搜索并播放: " << clean_song_name << "..." << std::endl;
    // TODO: 在这里集成你自己的 sqlite3 音乐搜索和播放逻辑
}

// 播放音频文件的函数
void play_audio(const std::string& filename) {
    std::cout << "[播放器] 正在播放AI语音: " << filename << "..." << std::endl;
    // 使用 mpg123 命令行工具播放mp3，-q 参数表示静默模式，减少不必要的输出
    std::string command = "mpg123 -q " + filename;
    system(command.c_str());
}

int main(int argc, char* argv[]) {
    // 1. 全局初始化讯飞SDK，这必须在所有其他讯飞功能调用之前完成
    SparkChain::SparkChainConfig *config = SparkChain::SparkChainConfig::builder();
    config->appID(APPID.c_str())
          ->apiKey(APIKEY.c_str())
          ->apiSecret(APISECRET.c_str())
          ->workDir("./");
    int ret = SparkChain::init(config);
    if (ret != 0) {
        std::cerr << "SparkChain SDK 初始化失败! 错误码: " << ret << std::endl;
        return -1;
    }

    // 2. 初始化我们创建的各个功能模块
    SparkClient spark(APPID, APIKEY, APISECRET);
    TTSClient tts(APPID, APIKEY, APISECRET);
    ASRClient asr(APPID, APIKEY, APISECRET); // ASR客户端同样需要凭证，但我们已在内部处理
    MicCapture mic;

    // 尝试打开默认的麦克风设备
    if (!mic.open("default")) {
        std::cerr << "麦克风初始化失败! 请检查设备是否连接或被占用。" << std::endl;
        SparkChain::unInit();
        return -1;
    }
    
    std::cout << "################################" << std::endl;
    std::cout << "##      智能音乐播放器 启动     ##" << std::endl;
    std::cout << "################################" << std::endl;

    std::string recognized_text;
    const std::string play_command = "播放";
    const std::string exit_command = "退出";

    // 3. 进入主交互循环
    while (true) {
        std::cout << "\n=======================================" << std::endl;
        std::cout << "请在5秒内说话，或说“退出”来结束程序..." << std::endl;
        
        mic.record(5); // 开始录制5秒钟的音频
        std::cout << "录音结束，正在识别..." << std::endl;

        const auto& audio_data = mic.get_audio_data();
        if (audio_data.empty()) {
            std::cerr << "错误: 没有录到有效的音频数据。" << std::endl;
            continue;
        }

        // 将录制的音频发送给ASR服务进行识别
        recognized_text = asr.transcribe(audio_data);
        mic.clear_data(); // 清除上一段录音数据

        // 如果识别结果包含错误信息，则打印并跳过
        if (recognized_text.rfind("ASR Error", 0) == 0) {
            std::cerr << recognized_text << std::endl;
            continue;
        }

        std::cout << ">>> 你说的是: \"" << recognized_text << "\"" << std::endl;

        // 4. 根据识别出的文本，进行逻辑判断
        if (recognized_text.find(exit_command) != std::string::npos) {
            std::cout << "收到退出指令。" << std::endl;
            break;
        } else if (recognized_text.rfind(play_command, 0) == 0) {
            // 如果指令以“播放”开头
            std::string song_to_play = recognized_text.substr(play_command.length());
            play_music(song_to_play);
        } else if (!recognized_text.empty()) {
            // 对于所有其他非空指令，我们认为是与AI对话
            std::cout << "AI正在思考中..." << std::endl;
            std::string llm_answer = spark.ask(recognized_text);
            
            if (llm_answer.empty() || llm_answer.rfind("Error", 0) == 0) {
                std::cerr << "获取AI回答失败! " << llm_answer << std::endl;
                continue;
            }
            std::cout << ">>> AI的回答: " << llm_answer << std::endl;

            std::cout << "AI正在合成语音..." << std::endl;
            std::string output_mp3 = "ai_speech.mp3";
            if (tts.synthesize(llm_answer, output_mp3)) {
                play_audio(output_mp3);
            } else {
                std::cerr << "语音合成失败!" << std::endl;
            }
        }
    }
    
    // 5. 退出前，逆初始化SDK释放资源
    SparkChain::unInit();
    std::cout << "程序已退出。" << std::endl;
    return 0;
}
