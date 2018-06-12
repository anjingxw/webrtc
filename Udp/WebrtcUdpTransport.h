//
//  WebrtcUdpTransport.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/2.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef WebrtcUdpTransport_hpp
#define WebrtcUdpTransport_hpp
#include "api/call/transport.h"
#include "UdpTransportProxy.h"
#include "call/call.h"
#include "rtc_base/asyncinvoker.h"
#include "rtc_base/sigslot.h"
#include "CallWraperBase.h"

class WebrtcUdpTransport:public WebrtcUdpTransportBase, public webrtc::Transport, public sigslot::has_slots<>{
public:
    enum TransportType {
        kAudio = 1,
        kVideo = 2,
    };
    uint16_t GetLocalHostPort() override;
    bool SetRemoteIPHostPort(const char* ip, uint16_t port) override;
    
    WebrtcUdpTransport(TransportType type, std::string  name, bool ipv6);
    void SetReceive(rtc::Thread* work_thread, webrtc::PacketReceiver* receive);
    
    bool SendRtp(const uint8_t* packet, size_t length, const webrtc::PacketOptions& options) override;
    bool SendRtcp(const uint8_t* packet, size_t length) override;
    
    bool SetRemoteAddress(const rtc::SocketAddress& dest);
    rtc::SocketAddress GetLocalAddress() const;
    rtc::SocketAddress GetRemoteAddress() const;
public:
    void OnReadPacket(rtc::CopyOnWriteBuffer* buffer,  const rtc::PacketTime& packet_time);
    void DeliverPacket(rtc::CopyOnWriteBuffer packet, const rtc::PacketTime& packet_time);
    
public:
    void SetInfobirdTransport(bool infobirdTransport) override;
    void AudioPlayload(int audioPlayload) override;
private:
    bool infobirdTransport_;
    int  audioPlayload_;
    int  seqTemp_;
    size_t  PreSendProcess(bool isRtp, uint8_t* rtpData, size_t rtpDataLen, uint8_t*& sendData, size_t& sendDataLen);
    size_t  PreRecvProcess(bool isRtp, int payloadType, uint8_t* recvData, size_t recvDataLen, uint8_t* rtpData, size_t& rtpDataLen, int& seq);
private:
    rtc::Thread* work_thread_;
    webrtc::PacketReceiver* receive_;
    TransportType  type_;
    rtc::AsyncInvoker invoker_;
    std::unique_ptr<UdpTransportProxy> transportProxy_;
    uint16_t  localPort;
};
#endif /* WebrtcUdpTransport_hpp */
