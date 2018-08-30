//
//  TabAudioDevice.hpp
//  webrtcosx
//
//  Created by anjingxw@126.com on 2018/6/15.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef TabAudioDevice_hpp
#define TabAudioDevice_hpp

#include <memory>
#include <string>
#include <vector>

#include "api/array_view.h"
#include "modules/audio_device/include/fake_audio_device.h"
#include "rtc_base/buffer.h"
#include "rtc_base/criticalsection.h"
#include "rtc_base/event.h"
#include "rtc_base/platform_thread.h"
#include "rtc_base/random.h"
#include "typedefs.h"  // NOLINT(build/include)

namespace  webrtc{ class EventTimerWrapper; }
class TabAudioDevice: public webrtc::FakeAudioDeviceModule{
public:
    class Capturer {
    public:
        virtual ~Capturer() {}
        virtual int SamplingFrequency() const = 0;
        virtual bool Capture(rtc::BufferT<int16_t>* buffer) = 0;
    };
public:
    TabAudioDevice(std::unique_ptr<Capturer> capturer);
    ~TabAudioDevice() override;
    
    int32_t Init() override;
    int32_t RegisterAudioCallback(webrtc::AudioTransport* callback) override;
    
    int32_t StartRecording() override;
    int32_t StopRecording() override;

    bool Recording() const override;
    
    // Blocks until the Recorder stops producing data.
    // Returns false if |timeout_ms| passes before that happens.
    bool WaitForRecordingEnd(int timeout_ms = rtc::Event::kForever);
private:
    static bool Run(void* obj);
    void ProcessAudio();
    const std::unique_ptr<Capturer> capturer_ RTC_GUARDED_BY(lock_);
    rtc::CriticalSection lock_;
    webrtc::AudioTransport* audio_callback_ RTC_GUARDED_BY(lock_);
    bool capturing_ RTC_GUARDED_BY(lock_);
    rtc::Event done_capturing_;
    
    std::vector<int16_t> playout_buffer_ RTC_GUARDED_BY(lock_);
    rtc::BufferT<int16_t> recording_buffer_ RTC_GUARDED_BY(lock_);
    
    std::unique_ptr<webrtc::EventTimerWrapper> tick_;
    rtc::PlatformThread thread_;
};

#endif /* TabAudioDevice_hpp */
