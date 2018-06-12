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
#import  "sdk/objc/Framework/Headers/WebRTC/RTCAudioSessionConfiguration.h"
#import  "WebRTC/RTCVideoCodecH264.h"
#include "sdk/objc/Framework/Classes/VideoToolbox/objc_video_decoder_factory.h"
#include "sdk/objc/Framework/Classes/VideoToolbox/objc_video_encoder_factory.h"
#import  "sdk/objc/Framework/Classes/Video/avfoundationvideocapturer.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_decoder.h"
#include "CallWraperFactory.h"
#include "api/videosourceproxy.h"

#import  "RTCVideoRendererAdapter+Private.h"
#include "pc/videocapturertracksource.h"

cricket::VideoCapturer*  CallWraperPlatform::capturer_;
rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>  CallWraperPlatform::owned_source_ = nullptr;
rtc::VideoSourceInterface<webrtc::VideoFrame>* CallWraperPlatform::source_  = nullptr;
rtc::VideoSinkInterface<webrtc::VideoFrame>* CallWraperPlatform::render_ = nullptr;
void* CallWraperPlatform::_view = NULL;

CallWraperPlatform::CallWraperPlatform(bool onlyAudio):
onlyAudio_(onlyAudio){
}

CallWraperPlatform::~CallWraperPlatform(){
    RTCVideoRendererAdapter_  = nil;
}
void createInSign(){
    std::unique_ptr<cricket::VideoCapturer> capturer(CallWraperPlatform::capturer_);
    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source(webrtc::VideoCapturerTrackSource::Create(CallWraperFactory::Instance()->work_thread(), std::move(capturer), false));
   CallWraperPlatform::owned_source_ =  webrtc::VideoTrackSourceProxy::Create(CallWraperFactory::Instance()->signaling_thread(), CallWraperFactory::Instance()->work_thread(), source);
    
    CallWraperPlatform::source_ = CallWraperPlatform::owned_source_.get();
}
void deleteInSign(){
    CallWraperPlatform::owned_source_ = nullptr;
}

cricket::VideoCapturer*  CallWraperPlatform::Capturer(){
    if (!capturer_) {
        capturer_ = new webrtc::AVFoundationVideoCapturer();
    }
    if (owned_source_ == nullptr) {
        CallWraperFactory::Instance()->signaling_thread()->Invoke<void>(RTC_FROM_HERE, createInSign);
    }
    return  capturer_;
}

void  CallWraperPlatform::RestCapturer(){
    //capturer_->Stop();
    capturer_ = NULL;
    CallWraperFactory::Instance()->signaling_thread()->Invoke<void>(RTC_FROM_HERE, deleteInSign);
    source_  = NULL;
}

void CallWraperPlatform::SetUseBackCamera(bool useBackCamera){
    webrtc::AVFoundationVideoCapturer* capture = (webrtc::AVFoundationVideoCapturer*)Capturer();
    capture->SetUseBackCamera(useBackCamera);
}

bool CallWraperPlatform::GetUseBackCamera(){
    webrtc::AVFoundationVideoCapturer* capture = (webrtc::AVFoundationVideoCapturer*)Capturer();
    return  capture->GetUseBackCamera();
}

void*  CallWraperPlatform::GetCaptureSession(){
    CallWraperPlatform::Capturer();
    webrtc::AVFoundationVideoCapturer* capture = (webrtc::AVFoundationVideoCapturer*)capturer_;
    return (__bridge void *)capture->GetCaptureSession();
}

std::unique_ptr<webrtc::VideoEncoder> CallWraperPlatform::CreateVideoEncoder(const webrtc::SdpVideoFormat& format){
    auto video_encoder_factory_ = rtc::MakeUnique<webrtc::ObjCVideoEncoderFactory>([[RTCVideoEncoderFactoryH264 alloc] init]);
    return video_encoder_factory_->CreateVideoEncoder(format);
}

std::unique_ptr<webrtc::VideoDecoder> CallWraperPlatform::CreateVideoDecoder(const webrtc::SdpVideoFormat& format){
    auto video_decoder_factory_ = rtc::MakeUnique<webrtc::ObjCVideoDecoderFactory>([[RTCVideoDecoderFactoryH264 alloc] init]);
    return video_decoder_factory_->CreateVideoDecoder(format);
}

void* CallWraperPlatform::GetCaptureSession(cricket::VideoCapturer* ccapture){
    webrtc::AVFoundationVideoCapturer* capture = (webrtc::AVFoundationVideoCapturer*)ccapture;
    return (__bridge void *)capture->GetCaptureSession();
}

rtc::VideoSinkInterface<webrtc::VideoFrame>*  CallWraperPlatform::GetNativeVideoRenderer(void* view){
    if(view == NULL)
        view = _view;
    id<RTCVideoRenderer> o = (__bridge id<RTCVideoRenderer>)view;
    RTCVideoRendererAdapter_ =  [[RTCVideoRendererAdapter alloc] initWithNativeRenderer:o];
    return RTCVideoRendererAdapter_.nativeVideoRenderer;
}

void* CallWraperPlatform::GetView(){
    return _view;
}
void  CallWraperPlatform::SetView(void* view_){
    _view = view_;
}
