//
//  TabCapturer.cpp
//  webrtcosx
//
//  Created by anjingxw@126.com on 2018/6/15.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "TabCapturer.h"
#include "modules/audio_mixer/audio_mixer_impl.h"
#include "modules/audio_processing/audio_buffer.h"
#include "audio/utility/audio_frame_operations.h"

AudioFrameWithUse::AudioFrameWithUse(){
    frameUse = false;
    frame.Reset();
}

ReceiveSource::ReceiveSource(){
    rtc::CritScope cs(&lock_);
    audioFrameQueryUse_.push_back(new AudioFrameWithUse());
    audioFrameQueryUse_.push_back(new AudioFrameWithUse());
    audioFrameQueryUse_.push_back(new AudioFrameWithUse());
}

ReceiveSource::~ReceiveSource(){
    rtc::CritScope cs(&lock_);
    for (auto it = audioFrameQueryUse_.begin(); it != audioFrameQueryUse_.end(); it++) {
        delete *it;
    }
    audioFrameQueryUse_.clear();
    
}
void ReceiveSource::RecordNet(std::string path){
    if (path =="") {
        otherRecordFile_.reset();
    }else{
        otherRecordFile_.reset(new WavFileWriter(path, 8000));
    }
}

//对方的音频
void ReceiveSource::OnData(const webrtc::AudioSinkInterface::Data& audio){
    //only 1 channel
//    if (otherRecordFile_ == nullptr) {
//        otherRecordFile_.reset(new WavFileWriter("/Users/zhengzihui/Desktop/other.wav", 8000));
//    }
    rtc::CritScope cs(&lock_);
    if (audio.sample_rate == 8000) {
        otherRecordFile_->Render(audio.data, audio.samples_per_channel);
    
        AudioFrameWithUse* frameWU = getUnUseAudioFrame();
        if (frameWU == nullptr) {
            return;
        }
        frameWU->frame.UpdateFrame(audio.timestamp, audio.data, audio.samples_per_channel, audio.sample_rate, webrtc::AudioFrame::kUndefined , webrtc::AudioFrame::VADActivity::kVadUnknown);
        audioFrameQuery_.push(frameWU);
    }else{
        capture_resampler_.InitializeIfNeeded(audio.sample_rate, 8000 , 1);
        AudioFrameWithUse* frameWU = getUnUseAudioFrame();
        if (frameWU == nullptr) {
            return;
        }
        frameWU->frame.timestamp_ = audio.timestamp;
        frameWU->frame.sample_rate_hz_ = 8000;
        frameWU->frame.speech_type_ = webrtc::AudioFrame::kNormalSpeech;
        frameWU->frame.num_channels_ = 1;
        frameWU->frame.vad_activity_ =  webrtc::AudioFrame::VADActivity::kVadUnknown;
        frameWU->frame.samples_per_channel_ = (8000*audio.samples_per_channel)/audio.sample_rate;
        capture_resampler_.Resample(audio.data, audio.samples_per_channel, frameWU->frame.mutable_data(), 1000);
        audioFrameQuery_.push(frameWU);
    }
};

AudioFrameWithUse*  ReceiveSource::getUnUseAudioFrame(){
    for (auto it = audioFrameQueryUse_.begin(); it != audioFrameQueryUse_.end(); it++) {
        if (!(*it)->frameUse) {
            (*it)->frameUse =  true;
            return (*it);
        }
    }
    return nullptr;
}

webrtc::AudioMixer::Source::AudioFrameInfo ReceiveSource::GetAudioFrameWithInfo(int sample_rate_hz, webrtc::AudioFrame* audio_frame) {
    rtc::CritScope cs(&lock_);
    
    if (audioFrameQuery_.size() > 0) {
        AudioFrameWithUse*  frameWU = audioFrameQuery_.front();
        audio_frame->CopyFrom(frameWU->frame);
        frameWU->frameUse = false;
        audioFrameQuery_.pop();
        
        return webrtc::AudioMixer::Source::AudioFrameInfo::kNormal;
    }
    
    return webrtc::AudioMixer::Source::AudioFrameInfo::kMuted;
}


TabCapturer::TabCapturer(webrtc::AudioMixer::Source* source){
    source_ = source;
    mix_ = webrtc::AudioMixerImpl::Create();
    mix_->AddSource(this);
    mix_->AddSource(source);
    
    rtc::CritScope cs(&lock_);
    audioFrameQueryUse_.push_back(new AudioFrameWithUse());
    audioFrameQueryUse_.push_back(new AudioFrameWithUse());
    audioFrameQueryUse_.push_back(new AudioFrameWithUse());
    
    mixFrame.UpdateFrame(0, nullptr, 80, 8000, webrtc::AudioFrame::kNormalSpeech,  webrtc::AudioFrame::VADActivity::kVadUnknown);
    //mixRecordFile_.reset(new WavFileWriter("/Users/zhengzihui/Desktop/mix.wav",8000));
    //selfRecordFile_.reset(new WavFileWriter("/Users/zhengzihui/Desktop/self.wav",8000));
}

void TabCapturer::SaveRecordAudioBuffer(int sample_rate_hz, webrtc::AudioBuffer* buffer){
    rtc::CritScope cs(&lock_);
    AudioFrameWithUse* frameWU = getUnUseAudioFrame();
    if (frameWU == nullptr) {
        return;
    }
    frameWU->frame.UpdateFrame(0, nullptr, 80, 8000, webrtc::AudioFrame::kNormalSpeech,  webrtc::AudioFrame::VADActivity::kVadUnknown);
    if (sample_rate_hz == 8000) {
        for (int i = 0; i< buffer->num_frames(); i++) {
            frameWU->frame.mutable_data()[i] = buffer->channels()[0][i];
        }
        frameWU->frame.samples_per_channel_ = buffer->num_frames();
    }else{
        capture_resampler_.InitializeIfNeeded(sample_rate_hz, 8000 , 1);
        frameWU->frame.samples_per_channel_ = (8000*buffer->num_frames())/sample_rate_hz;
        capture_resampler_.Resample(buffer->channels()[0], buffer->num_frames(),frameWU->frame.mutable_data(),1000);
    }
    audioFrameQuery_.push(frameWU);
    
    
    if (selfRecordFile_ != nullptr) {
        selfRecordFile_->Render(frameWU->frame.data(), frameWU->frame.samples_per_channel_);
    }
}

webrtc::AudioMixer::Source::AudioFrameInfo TabCapturer::GetAudioFrameWithInfo(int sample_rate_hz, webrtc::AudioFrame* audio_frame) {
    if (audioFrameQuery_.size() > 0) {
        AudioFrameWithUse*  frameWU = audioFrameQuery_.front();
        audio_frame->CopyFrom(frameWU->frame);
        frameWU->frameUse = false;
        audioFrameQuery_.pop();
        
        return webrtc::AudioMixer::Source::AudioFrameInfo::kNormal;
    }
    
    return webrtc::AudioMixer::Source::AudioFrameInfo::kMuted;
}

int TabCapturer::SamplingFrequency() const{
    return 8000;
};

bool TabCapturer::Capture(rtc::BufferT<int16_t>* buffer){
    mix_->Mix(1, &mixFrame);
    
    if (mixFrame.muted()) {
        mixFrame.mutable_data();
    }
    
    if (mixRecordFile_ != nullptr) {
        mixRecordFile_->Render(mixFrame.data(), mixFrame.samples_per_channel_*mixFrame.num_channels_);
    }
    buffer->SetData(mixFrame.data(), mixFrame.samples_per_channel_*mixFrame.num_channels_);
    return true;
}


void TabCapturer::RecordMixToFile(std::string path){
    if (path =="") {
        mixRecordFile_.reset();
    }else{
        mixRecordFile_.reset(new WavFileWriter(path, 8000));
    }
}

void TabCapturer::RecordSelfToFile(std::string path){
    if (path =="") {
        selfRecordFile_.reset();
    }else{
        selfRecordFile_.reset(new WavFileWriter(path, 8000));
    }
}

AudioFrameWithUse*  TabCapturer::getUnUseAudioFrame(){
    for (auto it = audioFrameQueryUse_.begin(); it != audioFrameQueryUse_.end(); it++) {
        if (!(*it)->frameUse) {
            (*it)->frameUse =  true;
            return (*it);
        }
    }
    return nullptr;
}


void TabCapturer::EndRecord(){
    selfRecordFile_.reset();
    mixRecordFile_.reset();
}
