//
//  CallWraperBase.h
//  webrtc
//
//  Created by anjingxw@126.com on 2018/4/3.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef DCallWraperBase_h
#define DCallWraperBase_h
#include <string>
class DetectTransport{
public:
    virtual bool SendG711A(const uint8_t* packet,  size_t length) = 0;
};

class DetectCallWraperBase {
public:
    virtual void CreateCallAndAudioDevice() = 0;
    virtual void SetDetectTransport(DetectTransport* transport)= 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
};

extern "C" __attribute__((visibility("default"))) DetectCallWraperBase *GetDetectCallWraperBase();

#endif /* CallWraperBase_h */
