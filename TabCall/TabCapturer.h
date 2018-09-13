//
//  TabCapturer.hpp
//  webrtcosx
//
//  Created by anjingxw@126.com on 2018/6/15.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef TabCapturer_hpp
#define TabCapturer_hpp

#include "modules/audio_processing/include/audio_processing.h"
#include "TabAudioDevice.h"
#include "api/call/audio_sink.h"
#include "api/audio/audio_mixer.h"
#include "modules/audio_processing/audio_buffer.h"
#include "Dtmf/dtmf_inband_process.h"
#include "common_audio/wav_file.h"
#include "common_audio/resampler/include/push_resampler.h"

#include <queue>
#include <list>
class AudioFrameWithUse{
public:
    AudioFrameWithUse();
    webrtc::AudioFrame frame;
    bool frameUse;
};

class WavFileWriter{
public:
    WavFileWriter(std::string filename, int sampling_frequency_in_hz) : sampling_frequency_in_hz_(sampling_frequency_in_hz),
    wav_writer_(filename, sampling_frequency_in_hz, 1) {}

    int SamplingFrequency() const  {  return sampling_frequency_in_hz_; }
    bool Render(rtc::ArrayView<const int16_t> data)  { wav_writer_.WriteSamples(data.data(), data.size()); return true; }
    bool Render(const int16_t* samples, size_t num_samples)  { wav_writer_.WriteSamples(samples, num_samples); return true; }
private:
    int sampling_frequency_in_hz_;
    webrtc::WavWriter wav_writer_;
};

class ReceiveSource:public webrtc::AudioMixer::Source, public webrtc::AudioSinkInterface {
public:
    ReceiveSource();
    ~ReceiveSource();
    void OnData(const webrtc::AudioSinkInterface::Data& audio) override;

public:
    webrtc::AudioMixer::Source::AudioFrameInfo GetAudioFrameWithInfo(int sample_rate_hz, webrtc::AudioFrame* audio_frame) override;
    int Ssrc() const override{return 0;};
    int PreferredSampleRate()const override{return 8000;};
    
    void RecordNet(std::string path);
    
private:
    AudioFrameWithUse*  getUnUseAudioFrame();
    
    std::unique_ptr<WavFileWriter>  otherRecordFile_;
    webrtc::PushResampler<int16_t> capture_resampler_;
    std::queue<AudioFrameWithUse*>  audioFrameQuery_;
    std::list<AudioFrameWithUse*>  audioFrameQueryUse_;
    rtc::CriticalSection lock_;
};

class TabCapturer:public TabAudioDevice::Capturer, public webrtc::AudioMixer::Source, public OutAudioBuffer{
public:
    TabCapturer(webrtc::AudioMixer::Source* source);
    void SaveRecordAudioBuffer(int sample_rate_hz, webrtc::AudioBuffer* buffer) override;
    int SamplingFrequency() const override;
    bool Capture(rtc::BufferT<int16_t>* buffer) override;
    webrtc::AudioMixer::Source::AudioFrameInfo GetAudioFrameWithInfo(int sample_rate_hz, webrtc::AudioFrame* audio_frame) override;
    int Ssrc() const override{return 1;};
    int PreferredSampleRate()const override{return 8000;};
    
    void RecordMixToFile(std::string path);
    void RecordSelfToFile(std::string path);
    void EndRecord();
private:
    webrtc::AudioMixer::Source* source_;
    std::unique_ptr<WavFileWriter>  mixRecordFile_;
    std::unique_ptr<WavFileWriter>  selfRecordFile_;
    rtc::CriticalSection lock_;
    rtc::scoped_refptr<webrtc::AudioMixer> mix_;
    webrtc::PushResampler<int16_t> capture_resampler_;
    webrtc::AudioFrame  mixFrame;
    
    AudioFrameWithUse*  getUnUseAudioFrame();
    std::queue<AudioFrameWithUse*>  audioFrameQuery_;
    std::list<AudioFrameWithUse*>  audioFrameQueryUse_;
    
    
};

#endif /* TabCapturer_hpp */
