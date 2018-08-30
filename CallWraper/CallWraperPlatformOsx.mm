//
//  CallWraperPlatform.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/6.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "CallWraperPlatform.h"
#include "rtc_base/ptr_util.h"
#include "rtc_base/refcountedobject.h"
#include "api/video_codecs/video_decoder_factory.h"
#import  "WebRTC/RTCVideoCodecH264.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_decoder.h"
#include "CallWraperFactory.h"
#include "api/videosourceproxy.h"

#import  "RTCVideoRendererAdapter+Private.h"
#include "pc/videocapturertracksource.h"

cricket::VideoCapturer*  CallWraperPlatform::capturer_;
//rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>  CallWraperPlatform::owned_source_ = nullptr;
rtc::VideoSourceInterface<webrtc::VideoFrame>* CallWraperPlatform::source_  = nullptr;
rtc::VideoSinkInterface<webrtc::VideoFrame>* CallWraperPlatform::render_ = nullptr;
void* CallWraperPlatform::_view = NULL;

CallWraperPlatform::CallWraperPlatform(bool onlyAudio):
onlyAudio_(onlyAudio){
}

CallWraperPlatform::~CallWraperPlatform(){
}

void createInSign(){
}

void deleteInSign(){
}

cricket::VideoCapturer*  CallWraperPlatform::Capturer(){
    return  capturer_;
}

void  CallWraperPlatform::RestCapturer(){
}

void CallWraperPlatform::SetUseBackCamera(bool useBackCamera){
}

bool CallWraperPlatform::GetUseBackCamera(){
    return false;
}

void*  CallWraperPlatform::GetCaptureSession(){
    return nullptr;
}

std::unique_ptr<webrtc::VideoEncoder> CallWraperPlatform::CreateVideoEncoder(const webrtc::SdpVideoFormat& format){
    return std::unique_ptr<webrtc::VideoEncoder>();
}

std::unique_ptr<webrtc::VideoDecoder> CallWraperPlatform::CreateVideoDecoder(const webrtc::SdpVideoFormat& format){
    return std::unique_ptr<webrtc::VideoDecoder>();
}

void* CallWraperPlatform::GetCaptureSession(cricket::VideoCapturer* ccapture){
    return nullptr;
}

rtc::VideoSinkInterface<webrtc::VideoFrame>*  CallWraperPlatform::GetNativeVideoRenderer(void* view){
    return nullptr;
}

void* CallWraperPlatform::GetView(){
    return _view;
}
void  CallWraperPlatform::SetView(void* view_){
    _view = view_;
}
