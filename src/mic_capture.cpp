#include "mic_capture.h"
#include <iostream>

MicCapture::MicCapture() : _handle(nullptr), _params(nullptr) {}

MicCapture::~MicCapture() {
    if (_handle) {
        snd_pcm_close(_handle);
    }
    // **核心修正：释放之前在堆上分配的参数对象**
    if (_params) {
        snd_pcm_hw_params_free(_params);
    }
}

bool MicCapture::open(const char* device_name) {
    int rc = snd_pcm_open(&_handle, device_name, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        std::cerr << "无法打开录音设备: " << snd_strerror(rc) << std::endl;
        return false;
    }
    return configure();
}

bool MicCapture::configure() {
    // **核心修正：使用 malloc 在堆上分配内存，而不是在栈上**
    int rc = snd_pcm_hw_params_malloc(&_params);
    if (rc < 0) {
        std::cerr << "无法分配硬件参数结构: " << snd_strerror(rc) << std::endl;
        return false;
    }

    rc = snd_pcm_hw_params_any(_handle, _params);
    if (rc < 0) {
        std::cerr << "无法初始化硬件参数结构: " << snd_strerror(rc) << std::endl;
        return false;
    }

    snd_pcm_hw_params_set_access(_handle, _params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(_handle, _params, _channels);
    
    unsigned int rate = _sample_rate;
    rc = snd_pcm_hw_params_set_rate_near(_handle, _params, &rate, nullptr);
    if (rc < 0) {
        std::cerr << "无法设置采样率: " << snd_strerror(rc) << std::endl;
        return false;
    }
    if (rate != _sample_rate) {
        std::cerr << "警告: 实际采样率 " << rate << " 与期望值 " << _sample_rate << " 不同" << std::endl;
    }

    rc = snd_pcm_hw_params(_handle, _params);
    if (rc < 0) {
        std::cerr << "无法设置硬件参数: " << snd_strerror(rc) << std::endl;
        return false;
    }
    return true;
}

bool MicCapture::record(int duration_seconds) {
    if (!_handle) return false;
    clear_data();

    snd_pcm_uframes_t frames_per_period;
    snd_pcm_hw_params_get_period_size(_params, &frames_per_period, nullptr);
    
    // 确保 frames_per_period 是一个有效值
    if (frames_per_period == 0) {
        std::cerr << "错误: ALSA period size 为 0" << std::endl;
        return false;
    }
    
    int chunk_size = frames_per_period * _channels * 2; // 2 bytes/sample for S16_LE
    std::vector<char> chunk_buffer(chunk_size);
    
    // 计算需要读取多少个周期（chunk）
    long loops = (long)(_sample_rate / (double)frames_per_period) * duration_seconds;

    std::cout << "  (开始录音...)\n";
    for (long i = 0; i < loops; i++) {
        int rc = snd_pcm_readi(_handle, chunk_buffer.data(), frames_per_period);
        if (rc == -EPIPE) {
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(_handle);
        } else if (rc < 0) {
            std::cerr << "从设备读取错误: " << snd_strerror(rc) << std::endl;
        } else if (rc != (int)frames_per_period) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
        _buffer.insert(_buffer.end(), chunk_buffer.begin(), chunk_buffer.end());
    }
    std::cout << "  (录音结束)\n";
    
    // 录音结束后清空并准备下一次录音
    snd_pcm_drain(_handle);
    snd_pcm_prepare(_handle);

    return true;
}
    
const std::vector<char>& MicCapture::get_audio_data() const {
    return _buffer;
}

void MicCapture::clear_data() {
    _buffer.clear();
}
