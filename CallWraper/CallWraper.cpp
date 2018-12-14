//
//  CallWraper.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/5.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "CallWraper.h"

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
#include "CallEncoderStreamFactory.hpp"
//#include <android/log.h>

#ifdef WEBRTC_IOS
struct HMAC_CTX;
extern "C"{
    HMAC_CTX* HMAC_CTX_new(void){
        return nullptr;
    }
    void HMAC_CTX_free(HMAC_CTX *ctx){
        
    }
}

#endif

const uint32_t kAudioSendSsrc = 12345;// 0xDEADBEEF;
const uint32_t kReceiverLocalAudioSsrc = kAudioSendSsrc;
const uint8_t  kVideoSendPayloadType = 125;
const uint32_t kReceiverLocalVideoSsrc = 0x123456;
const uint32_t kVideoSendSsrc = 0x654321;
const uint8_t  kFlexfecPayloadType = 120;
const uint32_t kFlexfecSendSsrc = 0xBADBEEF;

CallWraper::CallWraper(rtc::Thread* work_thread, webrtc::RtcEventLog* event_log,
                       rtc::scoped_refptr<webrtc::AudioDecoderFactory> decoder_factory,
                       rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory,
                       bool ipv6):
only_audio_(false),
audio_codec_plname_("isac"),
loop_(false),
work_thread_(work_thread),
event_log_(event_log),
send_audio_device_(nullptr),
video_send_config_(nullptr),
video_send_stream_(nullptr),
video_receive_config_(nullptr),
video_receive_stream_(nullptr),
video_receive_fec_stream_(nullptr),
audio_send_config_(nullptr),
audio_send_stream_(nullptr),
audio_receive_stream_(nullptr),
apm_(nullptr),
decoder_factory_(decoder_factory),
encoder_factory_(encoder_factory),
audioTransport_(new WebrtcUdpTransport(WebrtcUdpTransport::kAudio, "auido", ipv6)),
videoTransport_(only_audio_?nullptr:new WebrtcUdpTransport(WebrtcUdpTransport::kVideo, "video", ipv6)),
callWraperPlatform_(new CallWraperPlatform(only_audio_)),
flex_fec_(false){
    renderer_ = NULL;
    //for test
//    rtc::SocketAddress addr = audioTransport_->GetLocalAddress();
//    addr.SetIP(ipv6?"0:0:0:0:0:0:0:1":"127.0.0.1");
//    audioTransport_->SetRemoteAddress(addr);
//
//    addr = videoTransport_->GetLocalAddress();
//    addr.SetIP(ipv6?"0:0:0:0:0:0:0:1":"127.0.0.1");
//    videoTransport_->SetRemoteAddress(addr);
    
    if (renderer_ == NULL) {
        renderer_ = CallWraperPlatform::render_;
    }
}

CallWraper::~CallWraper(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__SignalChannelNetworkStateDown, this));
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__Finish_W, this));
}

void CallWraper::Config(bool only_audio, const char* audio_codec_plname, bool call_out){
    only_audio_ = only_audio;
    audio_codec_plname_ = audio_codec_plname;
    call_out_ = call_out;
    
    audioTransport_->AudioPlayload(__GetPayload());
}

WebrtcUdpTransportBase*  CallWraper::AudioTransport(){
    return audioTransport_.get();
}

WebrtcUdpTransportBase*  CallWraper::VideoTransport(){
    return videoTransport_.get();
}

void CallWraper::CreateCallAndAudioDevice(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__CreateCallAndAudioDevice, this));
}

void CallWraper::SetVideoRender(rtc::VideoSinkInterface<webrtc::VideoFrame>* render){
    renderer_ = render;
}

void CallWraper::SetVideoRenderView(void* view){
    SetVideoRender(callWraperPlatform_->GetNativeVideoRenderer(view));
}

void CallWraper::Start(){
     work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__Start, this));
     work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__SignalChannelNetworkState, this));
#ifdef HAS_TABCALL
     StartTabCall();
#endif
}

void CallWraper::StartOnlySend(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__StartOnlySend, this));
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__SignalChannelNetworkState, this));
}

void CallWraper::StartOnlyRecv(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__StartOnlyRecv, this));
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__SignalChannelNetworkState, this));
}

bool CallWraper::StartCaptrue(){
    return work_thread_->Invoke<bool>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__StartCapturer, this));
}

void  CallWraper::Stop(){
    work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__Stop, this));
}

webrtc::AudioDeviceModule*  CallWraper::getAudioDeviceModule(){
    return send_audio_device_.get();
}

void CallWraper::__CreateCallAndAudioDevice(){
    webrtc::Call::Config callConfig(event_log_);
    send_audio_device_ = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kPlatformDefaultAudio);

    dtmfDtmfInbandProcess_ = new DtmfInbandProcess;
    dtmfDtmfInbandProcess_->Set(nullptr);
    std::unique_ptr<webrtc::CustomProcessing>  dtmfInbandProcess(dtmfDtmfInbandProcess_);
    apm_ = webrtc::AudioProcessingBuilder().SetCapturePostProcessing(std::move(dtmfInbandProcess)).Create();
    webrtc::adm_helpers::Init(send_audio_device_);
    webrtc::apm_helpers::Init(apm_);
    
    webrtc::AudioState::Config audio_state_config;
    audio_state_config.audio_mixer = webrtc::AudioMixerImpl::Create();
    audio_state_config.audio_processing = apm_;
    audio_state_config.audio_device_module = send_audio_device_;
    callConfig.audio_state = webrtc::AudioState::Create(audio_state_config);
    
    send_audio_device_->RegisterAudioCallback(callConfig.audio_state->audio_transport());
    
    //createcall
    webrtc::RtpTransportControllerSend*  rtpTransportControllerSend = new webrtc::RtpTransportControllerSend(webrtc::Clock::GetRealTimeClock(), callConfig.event_log);

    call_.reset(webrtc::Call::Create(callConfig, std::unique_ptr<webrtc::RtpTransportControllerSend>(rtpTransportControllerSend)));
    audioTransport_->SetReceive(work_thread_, call_->Receiver());
    videoTransport_->SetReceive(work_thread_, call_->Receiver());
    
    __CreateSendConfig();
    __CreateReceiveConfig();
    if(!only_audio_){
        //work_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&CallWraper::__CreateCapturer, this));
        __CreateCapturer();
        __CreateVideoStreams();
    }
    __CreateAudioStreams();
//    __Start();
//    __SignalChannelNetworkState();
}

void CallWraper::__CreateVideoEncoderConfig(){
    cricket::VideoCodec codec("H264");
    //cricket::VideoCodec codec("VP8");
    video_encoder_config_.min_transmit_bitrate_bps = 200*1000; //250*1000;//250k
    video_encoder_config_.max_bitrate_bps = 600*1000;//1000*1000;//; //1000k
    video_encoder_config_.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;
    video_encoder_config_.number_of_streams = 1;
    video_encoder_config_.video_stream_factory = new rtc::RefCountedObject<CallEncoderStreamFactory>(codec.name);//new rtc::RefCountedObject<cricket::EncoderStreamFactory>(codec.name, 56, 60, false, false);
//    webrtc::VideoCodecVP8 v8Setting = webrtc::VideoEncoder::GetDefaultVp8Settings();
//    video_encoder_config_.encoder_specific_settings =  new rtc::RefCountedObject<webrtc::VideoEncoderConfig::Vp8EncoderSpecificSettings>(v8Setting);
    
    webrtc::VideoCodecH264 h264_settings =  webrtc::VideoEncoder::GetDefaultH264Settings();
    h264_settings.frameDroppingOn = false;
    h264_settings.keyFrameInterval = 50;
    h264_settings.profile = webrtc::H264::kProfileBaseline;//kProfileMain;//kProfileConstrainedBaseline;
    video_encoder_config_.encoder_specific_settings =  new rtc::RefCountedObject<webrtc::VideoEncoderConfig::H264EncoderSpecificSettings>(h264_settings);
}


void CallWraper::__CreateSendConfig(){
    //audio
    audio_send_config_ = webrtc::AudioSendStream::Config(audioTransport_.get());
    audio_send_config_.rtp.ssrc = kAudioSendSsrc;
    audio_send_config_.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(__GetPayload(), __GetSdpAudioFormat());
    audio_send_config_.encoder_factory = encoder_factory_;

    //video
    if(!only_audio_){
        video_send_config_ = webrtc::VideoSendStream::Config(videoTransport_.get());
        
        //const webrtc::SdpVideoFormat format("VP8", {});
        const webrtc::SdpVideoFormat format("H264", {});
        video_encoder_ = CallWraperPlatform::CreateVideoEncoder(format);
        video_send_config_.encoder_settings.encoder = video_encoder_.get();
        //RTC_LOG(LS_WARNING) << static_cast<void*>(video_send_config_.encoder_settings.encoder);
        //video_send_config_.encoder_settings.payload_name = "VP8";
        video_send_config_.encoder_settings.payload_name = "H264";
        video_send_config_.encoder_settings.payload_type = kVideoSendPayloadType;
        video_send_config_.rtp.ssrcs.push_back(call_out_?kVideoSendSsrc:kReceiverLocalVideoSsrc);
        video_send_config_.rtp.nack.rtp_history_ms = 1000;
        if (flex_fec_) {
            video_send_config_.rtp.flexfec.payload_type = kFlexfecPayloadType;
            video_send_config_.rtp.flexfec.protected_media_ssrcs = video_send_config_.rtp.ssrcs;
            video_send_config_.rtp.flexfec.ssrc = kFlexfecSendSsrc;
        }
    }
}

void CallWraper::__CreateReceiveConfig(){
    audio_receive_config_ =  webrtc::AudioReceiveStream::Config();
    audio_receive_config_.rtp.local_ssrc = kReceiverLocalAudioSsrc;
    audio_receive_config_.rtcp_send_transport = audioTransport_.get();
    audio_receive_config_.rtp.remote_ssrc = audio_send_config_.rtp.ssrc;
    audio_receive_config_.decoder_factory = decoder_factory_;
    audio_receive_config_.decoder_map = {{__GetPayload(),__GetSdpAudioFormat()}};

    if(!only_audio_){
        video_receive_config_ = webrtc::VideoReceiveStream::Config(videoTransport_.get());
        video_receive_config_.rtp.remb = false;
        video_receive_config_.rtp.transport_cc = true;
        video_receive_config_.rtp.local_ssrc = video_send_config_.rtp.ssrcs[0];
        //video_receive_config_.rtp.nack.rtp_history_ms = 1000;
        video_receive_config_.rtp.protected_by_flexfec = flex_fec_;
        video_receive_config_.rtp.remote_ssrc = !call_out_?kVideoSendSsrc:kReceiverLocalVideoSsrc;
        for (const webrtc::RtpExtension& extension : video_send_config_.rtp.extensions)
            video_receive_config_.rtp.extensions.push_back(extension);
        
        if (loop_) {
             video_receive_config_.rtp.local_ssrc = !call_out_?kVideoSendSsrc:kReceiverLocalVideoSsrc;
            video_receive_config_.rtp.remote_ssrc = video_send_config_.rtp.ssrcs[0];
        }
        
        video_receive_config_.renderer = this;
        video_receive_config_.render_delay_ms =  500;
        video_receive_config_.target_delay_ms = 100;
        //const webrtc::SdpVideoFormat format("VP8", {});
        const webrtc::SdpVideoFormat format("H264", {});
        video_decoder_ = CallWraperPlatform::CreateVideoDecoder(format);
        webrtc::VideoReceiveStream::Decoder decoder;
        decoder.decoder = video_decoder_.get();
        decoder.payload_name = video_send_config_.encoder_settings.payload_name ;
        decoder.payload_type = video_send_config_.encoder_settings.payload_type;
        video_receive_config_.decoders.push_back(decoder);
    }
}

void  CallWraper::__CreateCapturer(){
    //capturer_.reset(CallWraperPlatform::Capturer());
    //capturer_ = CallWraperPlatform::Capturer();
}

bool CallWraper::__StartCapturer(){
    __CreateCapturer();
    cricket::VideoFormat format(320,240,1000*1000*1000/10,cricket::FOURCC_NV12);
    //cricket::VideoFormat format(640,480,1000*1000*1000/10,cricket::FOURCC_NV12);
    cricket::VideoCapturer*  test = CallWraperPlatform::Capturer();
    cricket::CaptureState state = test->Start(format);
    return (state == cricket::CaptureState::CS_RUNNING);
}

void CallWraper::__SignalChannelNetworkState(){
    call_->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);
    call_->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkUp);
}

void CallWraper::__SignalChannelNetworkStateDown(){
    if(!call_)
        return;
    call_->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkDown);
    call_->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkDown);
}

void CallWraper::__CreateVideoStreams(){
    __CreateVideoEncoderConfig();
    video_send_stream_ = call_->CreateVideoSendStream(video_send_config_.Copy(), video_encoder_config_.Copy());
    video_send_stream_->SetSource(CallWraperPlatform::source_, webrtc::VideoSendStream::DegradationPreference::kMaintainResolution);

    video_receive_stream_ =  call_->CreateVideoReceiveStream(video_receive_config_.Copy());

    if (flex_fec_) {
        
        webrtc::FlexfecReceiveStream::Config video_receive_fec_config_(videoTransport_.get());
        video_receive_fec_config_.payload_type = kFlexfecPayloadType;
        video_receive_fec_config_.remote_ssrc = kFlexfecSendSsrc;
        video_receive_fec_config_.protected_media_ssrcs = {video_receive_config_.rtp.remote_ssrc };
        video_receive_fec_config_.local_ssrc = kReceiverLocalVideoSsrc;
        for (const webrtc::RtpExtension& extension : video_send_config_.rtp.extensions)
            video_receive_fec_config_.rtp_header_extensions.push_back(extension);
        video_receive_fec_stream_ = call_->CreateFlexfecReceiveStream(video_receive_fec_config_);
        video_receive_stream_->AddSecondarySink(video_receive_fec_stream_);
    }
}

void CallWraper::__CreateAudioStreams(){
    audio_send_stream_ = call_->CreateAudioSendStream(audio_send_config_);
    audio_receive_stream_ = call_->CreateAudioReceiveStream(audio_receive_config_);
        audio_receive_stream_->SetSink(this);
}

void CallWraper::__Start(){
    if (video_send_stream_)
       video_send_stream_->Start();
    
    if (video_receive_stream_) {
        video_receive_stream_->Start();
    }
    
    if (audio_send_stream_) {
        audio_send_stream_->Start();
    }
    
    if (audio_receive_stream_) {
        audio_receive_stream_->Start();
    }
}

void CallWraper::__Stop(){
    if (video_receive_stream_) {
        video_receive_stream_->Stop();
    }
    if (video_send_stream_)
        video_send_stream_->Stop();
    
    if (audio_receive_stream_) {
        audio_receive_stream_->Stop();
    }
    
    if (audio_send_stream_) {
        audio_send_stream_->Stop();
    }
}

void CallWraper::__StartOnlySend(){
    if (video_send_stream_)
        video_send_stream_->Start();
    
    if (video_receive_stream_) {
        video_receive_stream_->Stop();
    }
    
    if (audio_send_stream_) {
        audio_send_stream_->Start();
    }
    
    if (audio_receive_stream_) {
        audio_receive_stream_->Stop();
    }
}
void CallWraper::__StartOnlyRecv(){
    if (video_send_stream_)
        video_send_stream_->Stop();
    
    if (video_receive_stream_) {
        video_receive_stream_->Start();
    }
    
    if (audio_send_stream_) {
        audio_send_stream_->Stop();
    }
    
    if (audio_receive_stream_) {
        audio_receive_stream_->Start();
    }
}

void CallWraper::__DestroyStreams(){
    if (video_send_stream_)
        call_->DestroyVideoSendStream(video_send_stream_);
    
    if (video_receive_stream_) {
        if(video_receive_fec_stream_){
            video_receive_stream_->RemoveSecondarySink(video_receive_fec_stream_);
            call_->DestroyFlexfecReceiveStream(video_receive_fec_stream_);
        }
        call_->DestroyVideoReceiveStream(video_receive_stream_);
    }
    
    if (audio_send_stream_) {
        call_->DestroyAudioSendStream(audio_send_stream_);
    }
    
    if (audio_receive_stream_) {
        call_->DestroyAudioReceiveStream(audio_receive_stream_);
    }
}
void CallWraper::__Finish_W(){
    __DestroyStreams();
    audioTransport_.reset();
    videoTransport_.reset();
    CallWraperPlatform::RestCapturer();
    send_audio_device_.release();
    dtmfDtmfInbandProcess_ = NULL;
#ifdef HAS_TABCALL
    tabCall_.reset();
#endif
    call_.reset();
}

int CallWraper::__GetPayload(){
    if (rtc::ascicmp(audio_codec_plname_.c_str(), "g729") == 0) {
        return 18;
    }else if(rtc::ascicmp(audio_codec_plname_.c_str(), "g711") == 0){
        return 8;
    }
    else if(rtc::ascicmp(audio_codec_plname_.c_str(), "ilbc") == 0){
        return 102;
    }
    return 103;
}

webrtc::SdpAudioFormat CallWraper::__GetSdpAudioFormat(){
    if (rtc::ascicmp(audio_codec_plname_.c_str(), "g729") == 0) {
        return {"g729", 8000, 1};
    }else if(rtc::ascicmp(audio_codec_plname_.c_str(), "g711") == 0){
        return {"PCMA", 8000, 1, {{"ptime", "60"}}};
    }
    else if(rtc::ascicmp(audio_codec_plname_.c_str(), "ilbc") == 0){
        return {"ilbc", 8000, 1};
    }
    return {"isac", 16000, 1};
}

bool CallWraper::SendTelephoneEvent(int payload_type, int payload_frequency, int event, int duration_ms){
    if(dtmfDtmfInbandProcess_){
        dtmfDtmfInbandProcess_->SendTelephoneEventInband((unsigned char)event, 160, 10);
    }
    return true;
    //
    //return work_thread_->Invoke<bool>(RTC_FROM_HERE, rtc::Bind(&CallWraper::SendTelephoneEvent_w,this,payload_type, payload_frequency, event, duration_ms));
}
bool CallWraper::SendTelephoneEvent_w(int payload_type, int payload_frequency, int event, int duration_ms){
    if (audio_send_stream_) {
        audio_send_stream_->SendTelephoneEvent(payload_type, payload_frequency, event, duration_ms);
    }
    return false;
}

bool CallWraper::SetInputMute_w(bool mute){
    if (audio_send_stream_) {
        audio_send_stream_->SetMuted(mute);
    }
    return false;
}

bool CallWraper::SetInputMute(bool mute){
    return work_thread_->Invoke<bool>(RTC_FROM_HERE, rtc::Bind(&CallWraper::SetInputMute_w,this, mute));
}

int CallWraper::GetSoundQuality(){
    if (audio_receive_stream_) {
        webrtc::AudioReceiveStream::Stats stat = work_thread_->Invoke<webrtc::AudioReceiveStream::Stats>(RTC_FROM_HERE, rtc::Bind(&webrtc::AudioReceiveStream::GetStats,audio_receive_stream_));
        if (stat.packets_rcvd == 0) {
            return 0;
        }
        
        if(stat.delay_estimate_ms <=100){
            return 3;
        }else if(stat.delay_estimate_ms <=500){
            return 2;
        }else{
            return  1;
        }
    }
    
//    if(video_receive_stream_){
//        webrtc::VideoReceiveStream::Stats stat = work_thread_->Invoke<webrtc::VideoReceiveStream::Stats>(RTC_FROM_HERE, rtc::Bind(&webrtc::VideoReceiveStream::GetStats, video_receive_stream_));
//
//        RTC_LOG(LS_WARNING) << stat.ToString(0);
//    }
    return 0;
}

void CallWraper::OnFrame(const webrtc::VideoFrame& frame){
    //RTC_LOG(LS_WARNING) <<"CallWraper::OnFrame_"<<static_cast<void*>(renderer_);
    if (renderer_) {
        renderer_->OnFrame(frame);
    }
    //RTC_LOG(LS_WARNING) << static_cast<void*>(renderer_)<<"end";
}
void CallWraper::OnDiscardedFrame(){
    if (renderer_) {
        renderer_->OnDiscardedFrame();
    }
}
//const char* ip, uint16_t port
void CallWraper::OnData(const webrtc::AudioSinkInterface::Data& audio){
#ifdef HAS_TABCALL
    if(tabCall_ != nullptr && tabCall_->sink() != nullptr){
        tabCall_->sink()->OnData(audio);
    }
#endif
}

#ifdef HAS_TABCALL
void CallWraper::StartTabCall(){
    if (tabCall_ != nullptr) {
        return;
    }
    tabCall_.reset(new TabCallWraper(work_thread_, event_log_, encoder_factory_, false));
    tabCall_->CreateCallAndAudioDevice();
    dtmfDtmfInbandProcess_->Set(tabCall_->getTabCapturer());
    tabCall_->StartOnlySend();
}

bool CallWraper::AddTabToOther(const char* ip, uint16_t port){
    StartTabCall();
    TabAudioUdpTransport* transport  = tabCall_->transport();
    return  transport->AddRemoteIPHostPort(ip, port);
}

bool CallWraper::RemoveTabToOther(const char* ip, uint16_t port){
    StartTabCall();
    TabAudioUdpTransport* transport  = tabCall_->transport();
    if(ip == NULL || port == 0){
        transport->ClearRemoteIPHostPort();
        return true;
    }
    
    transport->DeleteRemoteIPHostPort(ip, port);
    return true;
}


bool CallWraper::PlayWav(const char* file_path){
    if(dtmfDtmfInbandProcess_){
        return dtmfDtmfInbandProcess_->PlayWav(file_path);
    }
    return  false;
}

bool CallWraper::StopPlayWav(){
    if(dtmfDtmfInbandProcess_){
        return dtmfDtmfInbandProcess_->StopPlayWav();
    }
    return true;
}

bool CallWraper::Record2File(const char* local, const char* net,  const char* mix){
    StartTabCall();
    TabCapturer * cap = tabCall_->getTabCapturer();
    if (cap) {
        if (!mix) {
            cap->RecordMixToFile(mix);
        }
        if (!local) {
            cap->RecordSelfToFile(local);
        }
    }
    if (!net) {
        ReceiveSource* source = tabCall_->receiveSource();
        if (source) {
            source->RecordNet(net);
        }
    }
    return true;
}

bool CallWraper::StopRecord2File(){
    StartTabCall();
    TabCapturer * cap = tabCall_->getTabCapturer();
    if (cap) {
        cap->EndRecord();
    }
    ReceiveSource* source = tabCall_->receiveSource();
    if (source) {
        source->RecordNet("");
    }
    return true;
}
#endif
