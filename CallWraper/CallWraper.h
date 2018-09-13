//
//  CallWraper.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/5.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef CallWraper_hpp
#define CallWraper_hpp
#include "CallWraperBase.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "call/call.h"
#include "rtc_base/thread.h"
#include "api/call/transport.h"
#include "media/base/videocapturer.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "CallWraperPlatform.h"
#include "TabCall/TabCallWraper.h"

class WebrtcUdpTransport;
class DtmfInbandProcess;

class CallWraper:public CallWraperBase, public rtc::VideoSinkInterface<webrtc::VideoFrame>, public webrtc::AudioSinkInterface{
public:
    CallWraper(rtc::Thread* work_thread, webrtc::RtcEventLog* event_log,
               rtc::scoped_refptr<webrtc::AudioDecoderFactory> decoder_factory,
               rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory,
               bool ipv6);
    ~CallWraper();

    void Config(bool only_audio, const char* audio_codec_plname, bool call_out = false) override;
    void CreateCallAndAudioDevice() override;
    void SetVideoRender(rtc::VideoSinkInterface<webrtc::VideoFrame>* render);
    void SetVideoRenderView(void* view) override;// extend id<RTCVideoRenderer>
    bool StartCaptrue() override;
    void Start() override;
    void Stop() override;
    
    void StartOnlySend() override;
    void StartOnlyRecv() override;
public:
    webrtc::AudioDeviceModule*  getAudioDeviceModule();
public:
    bool SendTelephoneEvent(int payload_type, int payload_frequency, int event, int duration_ms) override;
    bool SetInputMute(bool mute) override;
    int  GetSoundQuality() override;
    
public:
    WebrtcUdpTransportBase*  AudioTransport() override;
    WebrtcUdpTransportBase*  VideoTransport() override;
public:
    bool SendTelephoneEvent_w(int payload_type, int payload_frequency, int event, int duration_ms);
    bool SetInputMute_w(bool mute);
    
public: //rtc::VideoSinkInterface<webrtc::VideoFrame>
    void OnFrame(const webrtc::VideoFrame& frame) override;
    void OnDiscardedFrame() override;
private:
    bool only_audio_;
    std::string audio_codec_plname_;
    bool call_out_;
private:
    rtc::Thread* work_thread_;
    webrtc::RtcEventLog* event_log_;
    std::unique_ptr<webrtc::Call> call_;
    //视频  显示
    rtc::VideoSinkInterface<webrtc::VideoFrame>* renderer_;
    //音频  录制
    rtc::scoped_refptr<webrtc::AudioDeviceModule> send_audio_device_;
    //视频  发送
    webrtc::VideoSendStream::Config video_send_config_; //
    webrtc::VideoEncoderConfig video_encoder_config_;   //编码配置
    webrtc::VideoSendStream* video_send_stream_;
    //视频  接收
    webrtc::VideoReceiveStream::Config video_receive_config_;
    webrtc::VideoReceiveStream* video_receive_stream_;
    //音频  发送
    webrtc::AudioSendStream::Config audio_send_config_;
    webrtc::AudioSendStream* audio_send_stream_;
    //音频  接收
    webrtc::AudioReceiveStream::Config audio_receive_config_;
    webrtc::AudioReceiveStream* audio_receive_stream_;
    //音频  处理
    DtmfInbandProcess*  dtmfDtmfInbandProcess_;
    rtc::scoped_refptr<webrtc::AudioProcessing> apm_;
    
    rtc::scoped_refptr<webrtc::AudioDecoderFactory> decoder_factory_;
    rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory_;
    
    //视频编解码
    std::unique_ptr<webrtc::VideoEncoder> video_encoder_;
    std::unique_ptr<webrtc::VideoDecoder> video_decoder_;
    
    //传输
    std::unique_ptr<WebrtcUdpTransport>  audioTransport_;
    std::unique_ptr<WebrtcUdpTransport>  videoTransport_;
    
    //
    std::unique_ptr<CallWraperPlatform>  callWraperPlatform_;
    
    void __CreateCallAndAudioDevice();
    void __CreateSendConfig();
    void __CreateReceiveConfig();
    void __CreateCapturer();
    void __SignalChannelNetworkState();
    void __SignalChannelNetworkStateDown();
    void __CreateVideoEncoderConfig();
    void __CreateVideoStreams();
    void __CreateAudioStreams();
    bool __StartCapturer();
    void __Start();
    void __Stop();
    void __DestroyStreams();
    void __Finish_W();
    
    void __StartOnlySend();
    void __StartOnlyRecv();
    
    int __GetPayload();
    webrtc::SdpAudioFormat __GetSdpAudioFormat();
    
private:
    //const char* ip, uint16_t port
    void OnData(const webrtc::AudioSinkInterface::Data& audio) override;
#ifdef HAS_TABCALL
    bool AddTabToOther(const char* ip, uint16_t port) override;
    bool RemoveTabToOther(const char* ip, uint16_t port) override;
    void StartTabCall();
    std::unique_ptr<TabCallWraper> tabCall_;  //偷听
public:
    bool PlayWav(const char* file_path) override;
    bool StopPlayWav() override;
    bool Record2File(const char* local, const char* net,  const char* mix) override;
    bool StopRecord2File() override;
#endif
};

#endif /* CallWraper_hpp */
