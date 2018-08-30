//
//  WavReader.cpp
//  webrtcosx
//
//  Created by anjingxw@126.com on 2018/7/11.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "WavReader.h"
#include "common_audio/wav_file.h"
#include "audio/utility/audio_frame_operations.h"

WavReader::WavReader(std::string filePath, int sample_rate, int num_channels){
    sample_rate_ = sample_rate;
    num_channels_ = num_channels;
    wavReader_.reset(new webrtc::WavReader(filePath));
    has_data_ = true;
}

WavReader::~WavReader(){
    
    
}

int  WavReader::Get10msTone(int16_t output[320], uint16_t& outputSizeInSamples){
    int len10ms = sample_rate_/100;
    if (sample_rate_ == wavReader_->sample_rate() && num_channels_ == wavReader_->num_channels()) {
        auto ret = wavReader_->ReadSamples(len10ms, output);
        if (ret < outputSizeInSamples ) {
            has_data_ = false;
            return  -1;
        }else{
            outputSizeInSamples = ret;
        }
    }else{
        resampler_.InitializeIfNeeded( wavReader_->sample_rate(), sample_rate_, num_channels_);
        int16_t outputTemp[320] = {0};
        auto samples_per_channel = wavReader_->sample_rate()/100;
        auto len10msRead = samples_per_channel* wavReader_->num_channels();
        auto ret = wavReader_->ReadSamples(len10msRead, outputTemp);
        if (ret < len10msRead ) {
             has_data_ = false;
            return  -1;
        }else{
            if (wavReader_->num_channels() == 1 && num_channels_ == 2) {
                webrtc::AudioFrameOperations::MonoToStereo(outputTemp, samples_per_channel, outputTemp);
            }else if(wavReader_->num_channels() == 2 && num_channels_ == 1){
                webrtc::AudioFrameOperations::StereoToMono(outputTemp, samples_per_channel, outputTemp);
            }
            
            outputSizeInSamples = resampler_.Resample(outputTemp, samples_per_channel*num_channels_, output, 320);
        }
    }
    return 0;
}

bool WavReader::hasData(){
    return has_data_;
}
