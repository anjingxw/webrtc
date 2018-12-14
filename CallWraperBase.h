//
//  CallWraperBase.h
//  webrtc
//
//  Created by anjingxw@126.com on 2018/4/3.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef CallWraperBase_h
#define CallWraperBase_h
#include <string>
class WebrtcUdpTransportBase{
public:
    virtual uint16_t GetLocalHostPort() = 0;
    virtual bool SetRemoteIPHostPort(const char* ip, uint16_t port) = 0;
    virtual void SetInfobirdTransport(bool infobirdTransport) = 0;
    virtual void AudioPlayload(int audioPlayload) = 0;
};

class CallWraperBase {
public:
    virtual void Loop() = 0;
    virtual void Config(bool only_audio, const char* audio_codec_plname, bool call_out = false) = 0;
    virtual void CreateCallAndAudioDevice() = 0;
    virtual void SetVideoRenderView(void* view) = 0;// extend id<RTCVideoRenderer>
    virtual bool StartCaptrue() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    
    virtual void StartOnlySend() = 0;
    virtual void StartOnlyRecv() = 0;
    virtual WebrtcUdpTransportBase*  AudioTransport() = 0;
    virtual WebrtcUdpTransportBase*  VideoTransport() = 0;
    
    virtual bool SendTelephoneEvent(int payload_type, int payload_frequency, int event, int duration_ms) = 0;
    virtual bool SetInputMute(bool mute) = 0;
    virtual int  GetSoundQuality() = 0;
    virtual ~CallWraperBase(){};
public:
#ifdef HAS_TABCALL
    virtual bool AddTabToOther(const char* ip, uint16_t port) = 0;
    virtual bool RemoveTabToOther(const char* ip, uint16_t port) = 0;
    virtual bool PlayWav(const char* file_path){return false;};
    virtual bool StopPlayWav(){return false;};
    virtual bool Record2File(const char* local, const char* net,  const char* mix){return false;};
    virtual bool StopRecord2File(){return false;};
#endif
};

extern "C" __attribute__((visibility("default"))) CallWraperBase *GetCallWraperBase(bool ipv6);
extern "C" __attribute__((visibility("default"))) void SetCallWraperPlatformView(void* view);

extern "C" __attribute__((visibility("default"))) void InitLog(const char* path);

#endif /* CallWraperBase_h */
