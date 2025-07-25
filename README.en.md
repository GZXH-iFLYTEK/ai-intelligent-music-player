

# AI Intelligent Music Player

An intelligent music player integrating speech recognition, natural language processing, and speech synthesis.

## Features

- **Speech Recognition (ASR):** Converts speech to text  
- **Natural Language Processing (Spark):** Understands text commands and generates responses  
- **Speech Synthesis (TTS):** Converts text responses to speech output  
- **Audio Capture:** Records audio input through a microphone  

## Dependencies

- WebSocket&#43;&#43;  
- Boost.Asio  
- OpenSSL  
- PortAudio  
- nlohmann/json  

## Installation

1. Ensure all dependencies are installed  
2. Clone the repository: `git clone https://gitee.com/liu-jinnidie/ai-intelligent-music-player`  
3. Navigate into the project directory: `cd ai-intelligent-music-player`  
4. Build the project: `make`  

## Usage

1. Make sure all API keys and IDs are properly configured  
2. Run the program: `./intelligent_player`  
3. Use the microphone to record your voice: The program will automatically recognize the speech and play music according to the command  

## API Configuration

Before use, configure the following parameters in `src/intelligent_player.cpp`:  
- `APPID`  
- `APIKEY`  
- `APISECRET`  

## Main Components

- `ASRClient`: Handles speech recognition  
- `SparkClient`: Handles natural language processing  
- `TTSClient`: Handles speech synthesis  
- `MicCapture`: Handles audio capture  

## Example

Simply say into the microphone: "Play songs by Jay Chou", and the program will recognize the command and play the corresponding songs automatically.

## Contributions

Pull requests are welcome. For major changes, please open an issue to discuss the modifications first.

## License

This project is licensed under the MIT License. See the `LICENSE` file for more details.