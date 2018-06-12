//
//  ZZHAudioDecoderFactory.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/2/26.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "ZAudioDecoderFactory.h"
#include "common_types.h"  // NOLINT(build/include)
#include "rtc_base/ptr_util.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"

#include "modules/audio_coding/codecs/g729/audio_decoder_g729.h"

namespace zzh {

AudioDecoderFactory::AudioDecoderFactory():
    decoder_factory_(webrtc::CreateBuiltinAudioDecoderFactory()){
}

std::vector<webrtc::AudioCodecSpec> AudioDecoderFactory::GetSupportedDecoders(){
    std::vector<webrtc::AudioCodecSpec>  ret = decoder_factory_->GetSupportedDecoders();
    struct webrtc::AudioCodecSpec g729spec({{"G729", 8000, 1}, {8000, 1, 8000}});
    ret.push_back(g729spec);
    return ret;
}
    
bool AudioDecoderFactory::IsSupportedDecoder(const webrtc::SdpAudioFormat& format){
    const bool is_g729 = STR_CASE_CMP(format.name.c_str(), "G729") == 0;
    if (format.clockrate_hz == 8000 && format.num_channels >= 1 && is_g729) {
        return true;
    }
    return  decoder_factory_->IsSupportedDecoder(format);
}

std::unique_ptr<webrtc::AudioDecoder> AudioDecoderFactory::MakeAudioDecoder(const webrtc::SdpAudioFormat& format){
    const bool is_g728 = STR_CASE_CMP(format.name.c_str(), "G729") == 0;
    if (format.clockrate_hz == 8000 && format.num_channels >= 1 && is_g728) {
        return rtc::MakeUnique<webrtc::AudioDecoderG729>();
    }
    return  decoder_factory_->MakeAudioDecoder(format);
}
    
} //endof namespacezzh
