//
//  CallWraperPlatform.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/6.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef CallWraperPlatform_hpp
#define CallWraperPlatform_hpp
#include "media/base/videocapturer.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/mediastreaminterface.h"

#ifdef WEBRTC_IOS
#include "sdk/objc/Framework/Headers/WebRTC/RTCMacros.h"

RTC_FWD_DECL_OBJC_CLASS(RTCVideoRendererAdapter);
#endif

class CallWraperPlatform{
public:
    CallWraperPlatform(bool onlyAudio);
    ~CallWraperPlatform();
    
public:
    static cricket::VideoCapturer*  Capturer();
    static void  RestCapturer();
    static void* GetCaptureSession(); //AVCaptureSession*
    static std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& format);
    static std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(const webrtc::SdpVideoFormat& format);
    static cricket::VideoCapturer* capturer_;
    
    static void SetUseBackCamera(bool useBackCamera);
    static bool GetUseBackCamera();
    
    static void* GetView();
    static void  SetView(void* view);
    
private:
    static void* _view;
public:
    rtc::VideoSinkInterface<webrtc::VideoFrame>*  GetNativeVideoRenderer(void* view);
    void* GetCaptureSession(cricket::VideoCapturer* capture);
#ifdef WEBRTC_IOS
    static rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>  owned_source_;
private:
    RTCVideoRendererAdapter* RTCVideoRendererAdapter_;
    
#endif
    
#ifdef WEBRTC_ANDROID
    static void CreateVideoEncoderFactory(std::unique_ptr<webrtc::VideoEncoderFactory> ef);
    static void CreateVideoDecoderFactory(std::unique_ptr<webrtc::VideoDecoderFactory> df);
    static std::unique_ptr<webrtc::VideoEncoderFactory> video_encoder_factory_;
    static std::unique_ptr<webrtc::VideoDecoderFactory> video_decoder_factory_;
#endif
public:
    static rtc::VideoSourceInterface<webrtc::VideoFrame>* source_;
    static rtc::VideoSinkInterface<webrtc::VideoFrame>* render_;
private:
    bool onlyAudio_;
};

#endif /* CallWraperPlatform_hpp */
