### 全局介绍

`MySmartPlayer` 是一个功能丰富的智能音乐播放器项目，结合了语音交互、语音识别（ASR）、文本转语音（TTS）以及与星火大模型的对话功能。该项目允许用户通过语音指令控制音乐播放，同时可以与AI进行对话，为用户带来便捷、智能的交互体验。

#### 主要功能模块
1. **语音识别（ASR）**：借助 `ASRClient` 类，将麦克风录制的音频数据发送到语音识别服务，实现中文普通话的实时语音转文本。
2. **文本转语音（TTS）**：利用 `TTSClient` 类，把AI的回答文本转换为MP3格式的语音文件，并使用 `mpg123` 工具播放。
3. **智能对话**：通过 `SparkClient` 类与星火大模型交互，用户提问后可获取AI的回答。
4. **音乐播放控制**：用户发出“播放”的语音指令指定歌曲，系统会尝试搜索并播放相应音乐。
5. **麦克风录音**：`MicCapture` 类支持从默认麦克风设备录制音频，可指定录音时长。

#### 代码结构
- **`include` 目录**：存放项目的头文件，定义了各个功能模块的类和接口，如 `auth.h`、`spark_client.h`、`tts_client.h` 等。
- **`src` 目录**：包含项目的所有源文件，如 `intelligent_player.cpp` 是主程序入口，`asr_client.cpp`、`tts_client.cpp`、`spark_client.cpp` 分别实现了语音识别、文本转语音和智能对话的功能，`mic_capture.cpp` 负责麦克风录音。
- **`Makefile`**：用于编译项目，定义了编译选项、链接选项和目标文件，方便用户进行项目的编译和清理。

### 安装部署教程

#### 1. 环境准备
- **操作系统**：建议使用Linux系统，如Ubuntu。
- **依赖库安装**：
    - **ALSA库**：用于麦克风录音，提供音频设备的访问和配置功能。
    ```bash
    sudo apt-get install libasound2-dev
    ```
    - **OpenSSL库**：用于加密和认证，确保通信的安全性。
    ```bash
    sudo apt-get install libssl-dev
    ```
    - **Boost库**：提供多线程和网络编程的支持。
    ```bash
    sudo apt-get install libboost-all-dev
    ```
    - **nlohmann/json库**：用于处理JSON数据，方便与API进行交互。可以通过包管理器安装或从GitHub下载源码编译安装。
    ```bash
    sudo apt-get install nlohmann-json3-dev
    ```
    - **websocketpp库**：用于建立WebSocket连接，实现与服务端的实时通信。可以从GitHub下载源码并将其头文件添加到项目的包含路径中。
    - **mpg123**：用于播放MP3音频文件。
    ```bash
    sudo apt-get install mpg123
    ```
    - **sqlite3**：用于音乐搜索和播放逻辑（目前为占位功能）。
    ```bash
    sudo apt-get install libsqlite3-dev
    ```

#### 2. 下载项目代码
从代码仓库克隆项目到本地：
```bash
git clone <项目仓库地址>
cd MySmartPlayer
```

#### 3. 配置讯飞SDK路径
打开 `Makefile` 文件，修改 `SDK_PATH` 为你的讯飞SDK实际存放路径：
```makefile
# --- 请在这里配置你的讯飞SDK实际存放路径 ---
# 注意：我们增加了一层重复的目录名
SDK_PATH = /home/bobac3/SparkChain_Linux_SDK_2.0.0_rc1/SparkChain_Linux_SDK_2.0.0_rc1
```

#### 4. 配置API凭证
打开 `src/intelligent_player.cpp` 文件，将 `APPID`、`APIKEY` 和 `APISECRET` 替换为你自己的有效凭证：
```cpp
// --- 你的API凭证 ---
const std::string APPID     = "your_app_id";
const std::string APIKEY    = "your_api_key";
const std::string APISECRET = "your_api_secret";
```

#### 5. 编译项目
在项目根目录下执行以下命令编译项目：
```bash
make
```

#### 6. 运行程序
编译成功后，执行以下命令启动智能音乐播放器：
```bash
./intelligent_player
```

#### 7. 使用说明
- 程序启动后，会提示你在5秒内说话。你可以说“播放 <歌曲名>” 来播放指定的歌曲，或者提出其他问题与AI进行对话。
- 说“退出” 可以结束程序。

#### 8. 清理项目
如果需要清理编译生成的文件，可以执行以下命令：
```bash
make clean
```

通过以上步骤，你就可以完成 `MySmartPlayer` 项目的安装和部署，并开始使用智能音乐播放器的各项功能。