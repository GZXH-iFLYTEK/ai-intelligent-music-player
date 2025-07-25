#ifndef MIC_CAPTURE_H
#define MIC_CAPTURE_H

#include <string>
#include <vector>
#include <alsa/asoundlib.h>

class MicCapture {
public:
    MicCapture();
    ~MicCapture();

    bool open(const char* device_name = "default");
    bool record(int duration_seconds);
    const std::vector<char>& get_audio_data() const;
    void clear_data();

private:
    bool configure();

    snd_pcm_t* _handle;
    snd_pcm_hw_params_t* _params;
    std::vector<char> _buffer;
    unsigned int _sample_rate = 16000;
    unsigned int _channels = 1;
};

#endif // MIC_CAPTURE_H
