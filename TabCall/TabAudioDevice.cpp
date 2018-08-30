//
//  TabAudioDevice.cpp
//  webrtcosx
//
//  Created by anjingxw@126.com on 2018/6/15.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "TabAudioDevice.h"
#include "system_wrappers/include/event_wrapper.h"

TabAudioDevice::TabAudioDevice(std::unique_ptr<Capturer> capturer):
capturer_(std::move(capturer)),
audio_callback_(nullptr),
capturing_(false),
tick_(webrtc::EventTimerWrapper::Create()),
done_capturing_(true, true),
thread_(TabAudioDevice::Run, this, "TabAudioDevice") {
}

TabAudioDevice::~TabAudioDevice(){
    StopRecording();
    thread_.Stop();
}

int32_t TabAudioDevice::Init() {
    RTC_CHECK(tick_->StartTimer(true, 10));
    thread_.Start();
    thread_.SetPriority(rtc::kHighPriority);
    return 0;
}

int32_t TabAudioDevice::RegisterAudioCallback(webrtc::AudioTransport* callback) {
    rtc::CritScope cs(&lock_);
    RTC_DCHECK(callback || audio_callback_);
    audio_callback_ = callback;
    return 0;
}

bool TabAudioDevice::Run(void* obj){
    static_cast<TabAudioDevice*>(obj)->ProcessAudio();
    return true;
}

bool TabAudioDevice::Recording() const {
    rtc::CritScope cs(&lock_);
    return capturing_;
}

int32_t TabAudioDevice::StartRecording(){
    rtc::CritScope cs(&lock_);
    RTC_CHECK(capturer_);
    capturing_ = true;
    done_capturing_.Reset();
    return 0;
}

int32_t TabAudioDevice::StopRecording(){
    rtc::CritScope cs(&lock_);
    capturing_ = false;
    done_capturing_.Set();
    return 0;
}

bool TabAudioDevice::WaitForRecordingEnd(int timeout_ms) {
    return done_capturing_.Wait(timeout_ms);
}

void TabAudioDevice::ProcessAudio() {
    {
        rtc::CritScope cs(&lock_);
        if (capturing_) {
            // Capture 10ms of audio. 2 bytes per sample.
            const bool keep_capturing = capturer_->Capture(&recording_buffer_);
            uint32_t new_mic_level;
            if (recording_buffer_.size() > 0) {
                audio_callback_->RecordedDataIsAvailable(recording_buffer_.data(), recording_buffer_.size(), 2, 1,
                                                         capturer_->SamplingFrequency(), 0, 0, 0, false, new_mic_level);
            }
            if (!keep_capturing) {
                capturing_ = false;
                done_capturing_.Set();
            }
        }
    }
    tick_->Wait(WEBRTC_EVENT_INFINITE);
}
