//
//  DetectCallWraper.cpp
//  webrtcwraper
//
//  Created by anjingxw@126.com on 2018/3/8.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "DetectCallWraper.h"

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
#include "media/base/rtputils.h"

DetectCallWraper::DetectCallWraper(rtc::Thread* work_thread, webrtc::RtcEventLog* event_log,
                       rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory):
work_thread_(work_thread),
event_log_(event_log),
send_audio_device_(nullptr),
audio_send_config_(nullptr),
audio_send_stream_(nullptr),
apm_(nullptr),
encoder_factory_(encoder_factory)
{

}

DetectCallWraper::~DetectCallWraper(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&DetectCallWraper::__SignalChannelNetworkStateDown, this));
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&DetectCallWraper::__Finish_W, this));
}

void DetectCallWraper::CreateCallAndAudioDevice(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&DetectCallWraper::__CreateCallAndAudioDevice, this));
}

void DetectCallWraper::Start(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&DetectCallWraper::__Start, this));
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&DetectCallWraper::__SignalChannelNetworkState, this));
}

void  DetectCallWraper::Stop(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&DetectCallWraper::__Stop, this));
}

void DetectCallWraper::__CreateCallAndAudioDevice(){
    webrtc::Call::Config callConfig(event_log_);
    send_audio_device_ = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kPlatformDefaultAudio);
    
    apm_ = webrtc::AudioProcessingBuilder().Create();
    webrtc::adm_helpers::Init(send_audio_device_);
    webrtc::apm_helpers::Init(apm_);
    
    webrtc::AudioState::Config audio_state_config;
    audio_state_config.audio_mixer = webrtc::AudioMixerImpl::Create();
    audio_state_config.audio_processing = apm_;
    audio_state_config.audio_device_module = send_audio_device_;
    callConfig.audio_state = webrtc::AudioState::Create(audio_state_config);
    
    send_audio_device_->RegisterAudioCallback(callConfig.audio_state->audio_transport());
    
    //createcall
    webrtc::RtpTransportControllerSend*  rtpTransportControllerSend
    = new webrtc::RtpTransportControllerSend(webrtc::Clock::GetRealTimeClock(), callConfig.event_log);
    call_.reset(webrtc::Call::Create(callConfig, std::unique_ptr<webrtc::RtpTransportControllerSend>(rtpTransportControllerSend)));
    
    __CreateSendConfig();
    __CreateAudioStreams();
}

void DetectCallWraper::__CreateSendConfig(){
    //audio
    audio_send_config_ = webrtc::AudioSendStream::Config(this);
    audio_send_config_.rtp.ssrc = 12345;
    audio_send_config_.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(17, {"PCMA", 8000, 1});
    audio_send_config_.encoder_factory = encoder_factory_;
}

void DetectCallWraper::__SignalChannelNetworkState(){
    call_->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);
}

void DetectCallWraper::__SignalChannelNetworkStateDown(){
    call_->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkDown);
}

void DetectCallWraper::__CreateAudioStreams(){
    audio_send_stream_ = call_->CreateAudioSendStream(audio_send_config_);
}

void DetectCallWraper::__Start(){
    if (audio_send_stream_) {
        audio_send_stream_->Start();
    }
}

void DetectCallWraper::__Stop(){
    if (audio_send_stream_) {
        audio_send_stream_->Stop();
    }
}

void DetectCallWraper::__DestroyStreams(){
    if (audio_send_stream_) {
        call_->DestroyAudioSendStream(audio_send_stream_);
    }
}

void DetectCallWraper::__Finish_W(){
    __DestroyStreams();
    send_audio_device_.release();
    call_.reset();
}

bool DetectCallWraper::SendRtp(const uint8_t* packet,  size_t length, const webrtc::PacketOptions& options) {
    size_t rtpHeadLen;
    if(cricket::GetRtpHeaderLen(packet, length, &rtpHeadLen)){
        std::string error;
        if (audioTransport_) {
            audioTransport_->SendG711A( (packet + rtpHeadLen), length - rtpHeadLen);
        }
    }
    
    return true;
}

bool DetectCallWraper::SendRtcp(const uint8_t* packet, size_t length) {
    return true;
}


void DetectCallWraper::SetDetectTransport(DetectTransport* transport){
    audioTransport_ =  transport;
}

