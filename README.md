

# AI 智能音乐播放器

一个集成了语音识别、自然语言处理和语音合成的智能音乐播放器。

## 功能特性

- 语音识别 (ASR)：将语音转换为文本
- 自然语言处理 (Spark)：理解文本指令并生成响应
- 语音合成 (TTS)：将文本响应转换为语音输出
- 音频捕获：通过麦克风录制音频

## 依赖库

- WebSocket++
- Boost.Asio
- OpenSSL
- PortAudio
- nlohmann/json

## 安装

1. 确保安装了所有依赖库
2. 克隆仓库：`git clone https://gitee.com/liu-jinnidie/ai-intelligent-music-player`
3. 进入项目目录：`cd ai-intelligent-music-player`
4. 编译项目：`make`

## 使用

1. 确保已正确配置了所有API密钥和ID
2. 运行程序：`./intelligent_player`
3. 使用麦克风录制语音：程序会自动识别语音并根据指令播放音乐

## API 配置

在使用前，请在`src/intelligent_player.cpp`中配置以下参数：
- `APPID`
- `APIKEY`
- `APISECRET`

## 主要组件

- `ASRClient`：处理语音识别
- `SparkClient`：处理自然语言处理
- `TTSClient`：处理语音合成
- `MicCapture`：处理音频捕获

## 示例

只需对着麦克风说："播放周杰伦的歌"，程序会自动识别并播放相应歌曲。

## 贡献

欢迎提交Pull Request。对于重大更新，请先创建Issue讨论变更。

## 许可证

该项目使用MIT许可证，请参阅`LICENSE`文件获取详情。