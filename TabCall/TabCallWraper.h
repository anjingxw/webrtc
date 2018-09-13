//
//  CallWraper.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/5.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef TabCallWraper_hpp
#define TabCallWraper_hpp
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "call/call.h"
#include "rtc_base/thread.h"
#include "api/call/transport.h"
#include "media/base/videocapturer.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "CallWraper/CallWraperPlatform.h"
#include "TabCapturer.h"
#include "TabAudioDevice.h"
#include "TabUdpTransport.h"

class WebrtcUdpTransport;
class DtmfInbandProcess;

class TabCallWraper{
public:
    TabCallWraper(rtc::Thread* work_thread, webrtc::RtcEventLog* event_log,
               rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory,
               bool ipv6);
    ~TabCallWraper();
    
    TabAudioUdpTransport* transport(){return audioTransport_.get();};

    void CreateCallAndAudioDevice();
    void StartOnlySend() ;
    void Stop() ;
    TabCapturer*  getTabCapturer();
    webrtc::AudioSinkInterface* sink();
    ReceiveSource* receiveSource();
    
public:

private:
    rtc::Thread* work_thread_;
    webrtc::RtcEventLog* event_log_;
    std::unique_ptr<webrtc::Call> call_;
    //音频  录制
    rtc::scoped_refptr<webrtc::AudioDeviceModule> send_audio_device_;
    //音频  发送
    webrtc::AudioSendStream::Config audio_send_config_;
    webrtc::AudioSendStream* audio_send_stream_;
    rtc::scoped_refptr<webrtc::AudioProcessing> apm_;
    rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory_;
    //
    std::unique_ptr<ReceiveSource> receiveSource_;
    TabCapturer*  tabCapturer_;
    //传输
    std::unique_ptr<TabAudioUdpTransport>  audioTransport_;
    
    //
    std::unique_ptr<CallWraperPlatform>  callWraperPlatform_;
    void __CreateCallAndAudioDevice();
    void __CreateSendConfig();
    void __SignalChannelNetworkState();
    void __SignalChannelNetworkStateDown();
    void __CreateAudioStreams();
    void __Stop();
    void __DestroyStreams();
    void __Finish_W();
    void __StartOnlySend();
    int __GetPayload();
    webrtc::SdpAudioFormat __GetSdpAudioFormat();
};

#endif /* CallWraper_hpp */
