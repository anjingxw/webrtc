//
//  CallWraperFactory.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/5.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "CallWraperFactory.h"
#include "ZAudioDecoderFactory.h"
#include "ZAudioEncoderFactory.h"

CallWraperFactory* CallWraperFactory::Instance() {
    RTC_DEFINE_STATIC_LOCAL(CallWraperFactory, ls_callw_factory, ());
    return &ls_callw_factory;
}

CallWraperFactory::CallWraperFactory():event_log_(webrtc::RtcEventLog::CreateNull()),
decoder_factory_(new rtc::RefCountedObject<zzh::AudioDecoderFactory>()),
encoder_factory_(new rtc::RefCountedObject<zzh::AudioEncoderFactory>())
{
#ifdef WEBRTC_ANDROID
    rtc::ThreadManager::Instance()->WrapCurrentThread();
#endif
    {
        owned_work_thread_ = rtc::Thread::CreateWithSocketServer();
        owned_work_thread_->Start();
        work_thread_ = owned_work_thread_.get();
    }
    
    owned_signaling_thread_ = rtc::Thread::Create();
    owned_signaling_thread_->SetName("signaling_thread", NULL);
    owned_signaling_thread_->Start();
    signaling_thread_ = owned_signaling_thread_.get();
}

//CallWraperFactory::~CallWraperFactory(){
//    RTC_DCHECK(signaling_thread_->IsCurrent());
//    if (wraps_current_thread_)
//        rtc::ThreadManager::Instance()->UnwrapCurrentThread();
//}


rtc::Thread* CallWraperFactory::work_thread(){
    return work_thread_;
}

rtc::Thread* CallWraperFactory::signaling_thread(){
    return signaling_thread_;
}

std::unique_ptr<CallWraper> CallWraperFactory::CreateCallWraper(bool ipv6){
    return std::unique_ptr<CallWraper>(new CallWraper(work_thread_, event_log_.get(), decoder_factory_, encoder_factory_, ipv6));
}

std::unique_ptr<DetectCallWraper> CallWraperFactory::CreateDetectCallWraper(){
    return std::unique_ptr<DetectCallWraper>(new DetectCallWraper(work_thread_, event_log_.get(), encoder_factory_));
}

CallWraper* CallWraperFactory::PCreateCallWraper(bool ipv6){
    return new CallWraper(work_thread_, event_log_.get(), decoder_factory_, encoder_factory_, ipv6);
}

DetectCallWraper* CallWraperFactory::PCreateDetectCallWraper(){
    return  new DetectCallWraper(work_thread_, event_log_.get(), encoder_factory_);
}

DetectCallWraperBase *GetDetectCallWraperBase(){
    return CallWraperFactory::Instance()->PCreateDetectCallWraper();
}

CallWraperBase *GetCallWraperBase(bool ipv6){
    return CallWraperFactory::Instance()->PCreateCallWraper(ipv6);
}

void SetCallWraperPlatformView(void* view_){
    CallWraperPlatform::SetView(view_);
}
