//
//  CallWraper.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/5.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "TabCallWraper.h"

#include "call/video_config.h"
#include "media/engine/adm_helpers.h"
#include "media/engine/apm_helpers.h"
#include "rtc_base/refcountedobject.h"
#include "media/engine/webrtcvideoengine.h"
#include "call/rtp_transport_controller_send.h"
#include "modules/audio_mixer/audio_mixer_impl.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "modules/audio_device/include/audio_device.h"
#include "media/engine/webrtcvideoengine.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_decoder.h"
#include "CallWraperPlatform.h"
#include "rtc_base/stringutils.h"
#include "WebrtcUdpTransport.h"
#include "dtmf_inband_process.h"
#include "rtc_base/logging.h"
#include "TabAudioDevice.h"
//#include <android/log.h>

uint32_t kAudioSendSsrcA = 12345;

TabCallWraper::TabCallWraper(rtc::Thread* work_thread, webrtc::RtcEventLog* event_log,
                       rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory, bool ipv6):
work_thread_(work_thread),
event_log_(event_log),
send_audio_device_(nullptr),
audio_send_config_(nullptr),
audio_send_stream_(nullptr),
apm_(nullptr),
encoder_factory_(encoder_factory),
receiveSource_(nullptr),
tabCapturer_(nullptr),
audioTransport_(new TabAudioUdpTransport("tabAuido", ipv6))
{
    audioTransport_->SetInfobirdTransport(true);
    audioTransport_->AudioPlayload(__GetPayload());
}

TabCallWraper::~TabCallWraper(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&TabCallWraper::__SignalChannelNetworkStateDown, this));
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&TabCallWraper::__Finish_W, this));
}

void TabCallWraper::CreateCallAndAudioDevice(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&TabCallWraper::__CreateCallAndAudioDevice, this));
}


void TabCallWraper::StartOnlySend(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&TabCallWraper::__StartOnlySend, this));
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&TabCallWraper::__SignalChannelNetworkState, this));
}

void  TabCallWraper::Stop(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&TabCallWraper::__Stop, this));
}

webrtc::AudioSinkInterface* TabCallWraper::sink(){
    return receiveSource_.get();
}

TabCapturer* TabCallWraper::getTabCapturer(){
    return tabCapturer_;
}
ReceiveSource* TabCallWraper::receiveSource(){
    return receiveSource_.get();
}

void TabCallWraper::__CreateCallAndAudioDevice(){
    webrtc::Call::Config callConfig(event_log_);
    receiveSource_.reset(new ReceiveSource);
    tabCapturer_ = new TabCapturer(receiveSource_.get());
    std::unique_ptr<TabAudioDevice::Capturer> capturer(tabCapturer_);
    send_audio_device_ = rtc::scoped_refptr<TabAudioDevice>(new rtc::RefCountedObject<TabAudioDevice>(std::move(capturer)));
    apm_ = webrtc::AudioProcessingBuilder().Create();
    webrtc::apm_helpers::Init(apm_);
    send_audio_device_->Init();
    
    webrtc::AudioState::Config audio_state_config;
    audio_state_config.audio_mixer = webrtc::AudioMixerImpl::Create();
    audio_state_config.audio_processing = apm_;
    audio_state_config.audio_device_module = send_audio_device_;
    callConfig.audio_state = webrtc::AudioState::Create(audio_state_config);
    send_audio_device_->RegisterAudioCallback(callConfig.audio_state->audio_transport());
    
    //createcall
    webrtc::RtpTransportControllerSend*  rtpTransportControllerSend = new webrtc::RtpTransportControllerSend(webrtc::Clock::GetRealTimeClock(), callConfig.event_log);

    call_.reset(webrtc::Call::Create(callConfig, std::unique_ptr<webrtc::RtpTransportControllerSend>(rtpTransportControllerSend)));

    __CreateSendConfig();
    __CreateAudioStreams();
}

void TabCallWraper::__CreateSendConfig(){
    //audio
    audio_send_config_ = webrtc::AudioSendStream::Config(audioTransport_.get());
    audio_send_config_.rtp.ssrc = kAudioSendSsrcA;
    audio_send_config_.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(__GetPayload(), __GetSdpAudioFormat());
    audio_send_config_.encoder_factory = encoder_factory_;
}

void TabCallWraper::__SignalChannelNetworkState(){
    if(!call_)
        return;
    call_->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);
}

void TabCallWraper::__SignalChannelNetworkStateDown(){
    if(!call_)
        return;
    call_->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkDown);
}

void TabCallWraper::__CreateAudioStreams(){
    audio_send_stream_ = call_->CreateAudioSendStream(audio_send_config_);
}

void TabCallWraper::__Stop(){
    if (audio_send_stream_) {
        audio_send_stream_->Stop();
    }
}

void TabCallWraper::__StartOnlySend(){
    if (audio_send_stream_) {
        audio_send_stream_->Start();
    }
}

void TabCallWraper::__DestroyStreams(){
    if (audio_send_stream_) {
        call_->DestroyAudioSendStream(audio_send_stream_);
    }
}

void TabCallWraper::__Finish_W(){
    __DestroyStreams();
    audioTransport_.reset();
    tabCapturer_->EndRecord();
    send_audio_device_ = nullptr;
    call_.reset();
}

int TabCallWraper::__GetPayload(){
    return 102; //ilbc
}

webrtc::SdpAudioFormat TabCallWraper::__GetSdpAudioFormat(){
    return {"ilbc", 8000, 1};
}




