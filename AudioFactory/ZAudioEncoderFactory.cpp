//
//  ZZHAudioEncoderFactory.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/2/26.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "ZAudioEncoderFactory.h"
#include "common_types.h"  // NOLINT(build/include)
#include "rtc_base/ptr_util.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"


#include "modules/audio_coding/codecs/g729/audio_encoder_g729.h"
namespace zzh {

AudioEncoderFactory::AudioEncoderFactory()
    :encoder_factory_(webrtc::CreateBuiltinAudioEncoderFactory()){
}

std::vector<webrtc::AudioCodecSpec> AudioEncoderFactory::GetSupportedEncoders(){
    std::vector<webrtc::AudioCodecSpec> ret = encoder_factory_->GetSupportedEncoders();
    struct webrtc::AudioCodecSpec g729spec({{"G729", 8000, 1}, {8000, 1, 8000}});
    ret.push_back(g729spec);
    return ret;
}

rtc::Optional<webrtc::AudioCodecInfo> AudioEncoderFactory::QueryAudioEncoder(const webrtc::SdpAudioFormat& format){
    const bool is_g729 = STR_CASE_CMP(format.name.c_str(), "G729") == 0;
    if (format.clockrate_hz == 8000 && format.num_channels >= 1 && is_g729) {
        return webrtc::AudioCodecInfo({8000, 1, 8000});
    }
    return encoder_factory_->QueryAudioEncoder(format);
}

std::unique_ptr<webrtc::AudioEncoder> AudioEncoderFactory::MakeAudioEncoder(int payload_type, const webrtc::SdpAudioFormat& format) {
    const bool is_g729 = STR_CASE_CMP(format.name.c_str(), "G729") == 0;
    if (format.clockrate_hz == 8000 && format.num_channels >= 1 && is_g729) {
        webrtc::AudioEncoderG729::Config conf;
        conf.payload_type = payload_type;
        return rtc::MakeUnique<webrtc::AudioEncoderG729>(conf);
    }
    return encoder_factory_->MakeAudioEncoder(payload_type, format);
}
    
} //endof namespace zzh
