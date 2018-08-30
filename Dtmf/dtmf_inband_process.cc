//
//  dtmf_inband_process.cpp
//  webrtcwraper
//
//  Created by anjingxw@126.com on 2018/3/14.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "dtmf_inband_process.h"
#include "modules/audio_processing/audio_buffer.h"

void DtmfInbandProcess::Initialize(int sample_rate_hz, int num_channels){
    sample_rate_hz_  = sample_rate_hz;
    num_channels_ = num_channels;
    
    uint16_t frequency(0);
    _inbandDtmfGenerator.GetSampleRate(frequency);
    
    if (frequency != sample_rate_hz){
        _inbandDtmfGenerator.SetSampleRate((uint16_t) (sample_rate_hz));
        _inbandDtmfGenerator.ResetTone();
    }
};

void DtmfInbandProcess::Set(OutAudioBuffer* out){
    outAudioBuffer = out;
}

void DtmfInbandProcess::Process(webrtc::AudioBuffer* audio){
    if (outAudioBuffer != nullptr) {
        outAudioBuffer->SaveRecordAudioBuffer(sample_rate_hz_, audio);
    }
    
    if (_inbandDtmfQueue.PendingDtmf() &&
        !_inbandDtmfGenerator.IsAddingTone() &&
        _inbandDtmfGenerator.DelaySinceLastTone() > kMinTelephoneEventSeparationMs)
    {
        int8_t eventCode(0);
        uint16_t lengthMs(0);
        uint8_t attenuationDb(0);
        
        eventCode = _inbandDtmfQueue.NextDtmf(&lengthMs, &attenuationDb);
        _inbandDtmfGenerator.AddTone(eventCode, lengthMs, attenuationDb);
    }
    
    if (_inbandDtmfGenerator.IsAddingTone())
    {
        int16_t toneBuffer[320];
        uint16_t toneSamples(0);
        // Get 10ms tone segment and set time since last tone to zero
        if (_inbandDtmfGenerator.Get10msTone(toneBuffer, toneSamples) == -1){
            //WEBRTC_TRACE(kTraceWarning, kTraceVoice, VoEId(_instanceId, _channelId), "Channel::EncodeAndSend() inserting Dtmf failed");
            return;
        }
        
        // Replace mixed audio with DTMF tone.
        for (size_t sample = 0; sample < (size_t)audio->num_frames(); sample++){
            for (size_t channel = 0; channel < audio->num_channels();
                 channel++)
            {
                audio->channels()[channel][sample] = toneBuffer[sample];
            }
        }
    }
    else
    {
        // Add 10ms to "delay-since-last-tone" counter
        _inbandDtmfGenerator.UpdateDelaySinceLastTone();
    }
#ifdef HAS_TABCALL
    if (_wavReader && _wavReader->hasData()) {
        int16_t toneBuffer[320];
        uint16_t toneSamples(0);
        _wavReader->Get10msTone(toneBuffer, toneSamples);
        for (size_t sample = 0; sample < (size_t)audio->num_frames(); sample++){
            for (size_t channel = 0; channel < audio->num_channels();
                 channel++)
            {
                audio->channels()[channel][sample] = toneBuffer[sample];
            }
        }
    }
    else{
        _wavReader.reset();
    }
#endif
};

void DtmfInbandProcess::SendTelephoneEventInband(unsigned char eventCode, int lengthMs, int attenuationDb)
{
    _inbandDtmfQueue.AddDtmf(eventCode, lengthMs, attenuationDb);
}
std::string DtmfInbandProcess::ToString() const{
    return "DtmfInbandProcess";
};

#ifdef HAS_TABCALL
bool DtmfInbandProcess::PlayWav(const char* file){
    if(_wavReader){
        return false;
    }
    
    _wavReader.reset(new WavReader(file, sample_rate_hz_, num_channels_));
    return true;
}
bool DtmfInbandProcess::StopPlayWav(){
    _wavReader.reset(NULL);
    return true;
}
#endif
