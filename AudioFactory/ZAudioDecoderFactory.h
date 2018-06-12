//
//  ZZHAudioDecoderFactory.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/2/26.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef ZZHAudioDecoderFactory_hpp
#define ZZHAudioDecoderFactory_hpp

#include "rtc_base/scoped_ref_ptr.h"
#include "api/audio_codecs/audio_decoder_factory.h"

namespace zzh {
    
class AudioDecoderFactory : public webrtc::AudioDecoderFactory{
public:
    AudioDecoderFactory();
    
    std::vector<webrtc::AudioCodecSpec> GetSupportedDecoders() override;
    bool IsSupportedDecoder(const webrtc::SdpAudioFormat& format) override;
    std::unique_ptr<webrtc::AudioDecoder> MakeAudioDecoder(const webrtc::SdpAudioFormat& format) override;
private:
    rtc::scoped_refptr<webrtc::AudioDecoderFactory> decoder_factory_;
};
    
} //namespace zzh
#endif /* ZZHAudioDecoderFactory_hpp */
