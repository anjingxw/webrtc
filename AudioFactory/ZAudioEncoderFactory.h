//
//  ZZHAudioEncoderFactory.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/2/26.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef ZZHAudioEncoderFactory_hpp
#define ZZHAudioEncoderFactory_hpp

#include "rtc_base/scoped_ref_ptr.h"
#include "api/audio_codecs/audio_encoder_factory.h"

namespace zzh {
    
class AudioEncoderFactory : public webrtc::AudioEncoderFactory{
public:
    AudioEncoderFactory();
    std::vector<webrtc::AudioCodecSpec> GetSupportedEncoders() override;
    rtc::Optional<webrtc::AudioCodecInfo> QueryAudioEncoder(const webrtc::SdpAudioFormat& format) override;
    std::unique_ptr<webrtc::AudioEncoder> MakeAudioEncoder(int payload_type, const webrtc::SdpAudioFormat& format) override;
private:
    rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory_;
};//endof class AudioEncoderFactory
    
} //endof namespace zzh
#endif /* ZZHAudioEncoderFactory_hpp */
