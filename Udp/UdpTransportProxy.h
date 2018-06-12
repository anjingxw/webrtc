//
//  UdpTransportProxy.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/2.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef UdpTransportProxy_hpp
#define UdpTransportProxy_hpp
#include "p2p/base/udptransport.h"
#include "rtc_base/sigslot.h"

class UdpTransportProxy :public webrtc::UdpTransportInterface, public sigslot::has_slots<>, public rtc::MessageHandler {
public:
    UdpTransportProxy(rtc::Thread*, cricket::UdpTransport*);
    ~UdpTransportProxy();
    rtc::SocketAddress GetLocalAddress() const override;
    bool SetRemoteAddress(const rtc::SocketAddress& dest) override;
    rtc::SocketAddress GetRemoteAddress() const override;
    
    int SendDataEx(const uint8_t* data, size_t len);
    int SendData(const uint8_t* data, size_t len);
    int SendData_w(const uint8_t* data, size_t len);
    
    void OnReadyToSend(rtc::PacketTransportInternal* transport);
    void OnNetworkRouteChange(rtc::Optional<rtc::NetworkRoute> network_route);
    void OnWritableState(rtc::PacketTransportInternal* packet_transport);
    void OnSentPacket(rtc::PacketTransportInternal* packet_transport,
                      const rtc::SentPacket& sent_packet);
    void OnReadPacket(rtc::PacketTransportInternal* transport,
                      const char* data,
                      size_t len,
                      const rtc::PacketTime& packet_time,
                      int flags);
    
    void OnMessage(rtc::Message* msg) override;
    
    sigslot::signal2<rtc::CopyOnWriteBuffer*, const rtc::PacketTime&>  SignalPacketReceived;
protected:
    rtc::PacketTransportInternal* GetInternal() override;
    void __DeleteUdpTransport();
private:
    rtc::Thread* network_thread_;
    std::unique_ptr<cricket::UdpTransport> transport_;
};

#endif /* UdpTransportProxy_hpp */
