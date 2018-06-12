//
//  DetectCallWraper.hpp
//  webrtcwraper
//
//  Created by anjingxw@126.com on 2018/3/8.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef DetectCallWraper_hpp
#define DetectCallWraper_hpp

#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"

#include "call/call.h"
#include "rtc_base/thread.h"
#include "api/call/transport.h"
#include "logging/rtc_event_log/rtc_event_log.h"

class DetectTransport{
public:
    virtual bool SendG711A(const uint8_t* packet,  size_t length) = 0;
};

class DetectCallWraper: public webrtc::Transport {
public:
    DetectCallWraper(rtc::Thread* work_thread, webrtc::RtcEventLog* event_log,
               rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory);
    ~DetectCallWraper();
    
    void CreateCallAndAudioDevice();
    void Start();
    void Stop();
    
public:
    void SetDetectTransport(DetectTransport* transport);
public:
    bool SendRtp(const uint8_t* packet,  size_t length, const webrtc::PacketOptions& options) override;
    bool SendRtcp(const uint8_t* packet, size_t length) override;
    
private:
    rtc::Thread* work_thread_;
    webrtc::RtcEventLog* event_log_;
    std::unique_ptr<webrtc::Call> call_;

    //音频  录制
    rtc::scoped_refptr<webrtc::AudioDeviceModule> send_audio_device_;
    //音频  发送
    webrtc::AudioSendStream::Config audio_send_config_;
    webrtc::AudioSendStream* audio_send_stream_;
    //音频  处理
    rtc::scoped_refptr<webrtc::AudioProcessing> apm_;
    
    rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory_;
    
    //传输
    DetectTransport*  audioTransport_;
    
    void __CreateCallAndAudioDevice();
    void __CreateSendConfig();

    void __SignalChannelNetworkState();
    void __SignalChannelNetworkStateDown();
    void __CreateAudioStreams();
    void __Start();
    void __Stop();
    void __DestroyStreams();
    void __Finish_W();
};

#endif /* DetectCallWraper_hpp */
