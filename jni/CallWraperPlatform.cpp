//
//  service_jni.cpp
//  qtbservice
//
//  Created by zzh on 16/4/18.
//
//
#include <jni.h>
#include <android/log.h>
#include "CallWraperPlatform.h"
#include "rtc_base/ptr_util.h"
#include "rtc_base/refcountedobject.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_decoder.h"

#include "modules/video_coding/codecs/h264/include/h264.h"

cricket::VideoCapturer* CallWraperPlatform::capturer_ = nullptr;
std::unique_ptr<webrtc::VideoEncoderFactory> CallWraperPlatform::video_encoder_factory_= nullptr;
std::unique_ptr<webrtc::VideoDecoderFactory> CallWraperPlatform::video_decoder_factory_ = nullptr;
rtc::VideoSourceInterface<webrtc::VideoFrame>* CallWraperPlatform::source_ = NULL;
rtc::VideoSinkInterface<webrtc::VideoFrame>* CallWraperPlatform::render_ = NULL;
void* CallWraperPlatform::_view = NULL;

CallWraperPlatform::CallWraperPlatform(bool onlyAudio):
onlyAudio_(onlyAudio){
}

CallWraperPlatform::~CallWraperPlatform(){
}

cricket::VideoCapturer*  CallWraperPlatform::Capturer(){
    return  capturer_;
}

void  CallWraperPlatform::RestCapturer(){
    capturer_ = NULL;
}

void CallWraperPlatform::SetUseBackCamera(bool useBackCamera){
}

bool CallWraperPlatform::GetUseBackCamera(){
    return false;
}

void*  CallWraperPlatform::GetCaptureSession(){
    return NULL;
}

void CallWraperPlatform::CreateVideoEncoderFactory(std::unique_ptr<webrtc::VideoEncoderFactory> ef){
    video_encoder_factory_.swap(ef);
}
void CallWraperPlatform::CreateVideoDecoderFactory(std::unique_ptr<webrtc::VideoDecoderFactory> df){
    video_decoder_factory_.swap(df);
}

std::unique_ptr<webrtc::VideoEncoder> CallWraperPlatform::CreateVideoEncoder(const webrtc::SdpVideoFormat& format){
    std::unique_ptr<webrtc::VideoEncoder>  encode =  video_encoder_factory_->CreateVideoEncoder(format);
    if(encode != nullptr)
        return encode;
    cricket::VideoCodec codec(format);
    return webrtc::H264Encoder::Create(codec);
}

std::unique_ptr<webrtc::VideoDecoder> CallWraperPlatform::CreateVideoDecoder(const webrtc::SdpVideoFormat& format){
    std::unique_ptr<webrtc::VideoDecoder>  decode = video_decoder_factory_->CreateVideoDecoder(format);
    if (decode != nullptr) {
        return decode;
    }
    
    return webrtc::H264Decoder::Create();
}

void* CallWraperPlatform::GetCaptureSession(cricket::VideoCapturer* ccapture){
    return NULL;
}

rtc::VideoSinkInterface<webrtc::VideoFrame>*  CallWraperPlatform::GetNativeVideoRenderer(void* view){
    return render_;
}

void* CallWraperPlatform::GetView(){
    return _view;
}
void  CallWraperPlatform::SetView(void* view_){
    _view = view_;
}
