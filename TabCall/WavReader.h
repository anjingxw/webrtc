//
//  WavReader.hpp
//  webrtcosx
//
//  Created by anjingxw@126.com on 2018/7/11.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef WavReader_hpp
#define WavReader_hpp

#include "common_audio/wav_file.h"
#include "common_audio/resampler/include/push_resampler.h"
#include <memory>

class WavReader {
public:
    WavReader(std::string filePath, int sample_rate, int num_channels);
    ~WavReader();
    
    bool hasData();
    int Get10msTone(int16_t output[320], uint16_t& outputSizeInSamples);
public:
    std::unique_ptr<webrtc::WavReader>  wavReader_;
    webrtc::PushResampler<int16_t>      resampler_;
    int sample_rate_;
    int num_channels_;
    bool has_data_;
};
#endif /* WavReader_hpp */
