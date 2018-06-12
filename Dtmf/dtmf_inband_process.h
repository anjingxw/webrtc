//
//  dtmf_inband_process.hpp
//  webrtcwraper
//
//  Created by anjingxw@126.com on 2018/3/14.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef dtmf_inband_process_hpp
#define dtmf_inband_process_hpp

enum { kMinDtmfEventCode = 0 };         // DTMF digit "0"
enum { kMaxDtmfEventCode = 15 };        // DTMF digit "D"
enum { kMinTelephoneEventCode = 0 };    // RFC4733 (Section 2.3.1)
enum { kMaxTelephoneEventCode = 255 };  // RFC4733 (Section 2.3.1)
enum { kMinTelephoneEventDuration = 100 };
enum { kMaxTelephoneEventDuration = 60000 };       // Actual limit is 2^16
enum { kMinTelephoneEventAttenuation = 0 };        // 0 dBm0
enum { kMaxTelephoneEventAttenuation = 36 };       // -36 dBm0
enum { kMinTelephoneEventSeparationMs = 100 };     // Min delta time between two
// telephone events

#include "dtmf_inband.h"
#include "dtmf_inband_queue.h"

#include "modules/audio_processing/include/audio_processing.h"

class DtmfInbandProcess : public webrtc::CustomProcessing {
public:
    void Initialize(int sample_rate_hz, int num_channels) override;
    void Process(webrtc::AudioBuffer* audio) override;
    std::string ToString() const override;
    
    void SendTelephoneEventInband(unsigned char eventCode, int lengthMs, int attenuationDb);
private:
    int sample_rate_hz_;
    int num_channels_;
    webrtc::DtmfInbandQueue _inbandDtmfQueue;
    webrtc::DtmfInband _inbandDtmfGenerator;
};

#endif /* dtmf_inband_process_hpp */
