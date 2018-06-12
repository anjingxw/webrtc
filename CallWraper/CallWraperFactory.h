//
//  CallWraperFactory.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/5.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef CallWraperFactory_hpp
#define CallWraperFactory_hpp

#include "call/call.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_decoder.h"
#include "logging/rtc_event_log/rtc_event_log.h"

#include "CallWraper.h"
#include "DetectCallWraper.h"

class CallWraperFactory{
public:
    static CallWraperFactory* Instance();
    std::unique_ptr<CallWraper> CreateCallWraper(bool ipv6 = false);
    std::unique_ptr<DetectCallWraper> CreateDetectCallWraper();
    
    CallWraper* PCreateCallWraper(bool ipv6 = false);
    DetectCallWraper* PCreateDetectCallWraper();
    
    rtc::Thread* work_thread();
    rtc::Thread* signaling_thread();
protected:
    CallWraperFactory();
    
private:
    rtc::Thread* work_thread_;
    rtc::Thread* signaling_thread_;
    std::unique_ptr<webrtc::RtcEventLog> event_log_;
    std::unique_ptr<rtc::Thread> owned_work_thread_;
    std::unique_ptr<rtc::Thread> owned_signaling_thread_;
    rtc::scoped_refptr<webrtc::AudioDecoderFactory> decoder_factory_;
    rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory_;
};

#endif /* CallWraperFactory_hpp */
