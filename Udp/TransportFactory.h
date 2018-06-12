//
//  UdpTransport.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/1.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef TransportFactory_hpp
#define TransportFactory_hpp

#include "TransportBase.h"
#include "UdpTransportProxy.h"
    
class TransportFactory {
public:
    static TransportFactory* Instance();
public:
    TransportFactory();
    ~TransportFactory();
public:
    std::unique_ptr<UdpTransportProxy> CreateUdpTransport(std::string name, bool ipv6);
    std::unique_ptr<UdpTransportProxy> CreateUdpTransport_w(std::string name, bool ipv6);
private:
    rtc::Thread* network_thread_;
    rtc::NetworkManager* network_manager_;
    rtc::PacketSocketFactory* socket_factory_;
    
    std::unique_ptr<rtc::Thread> owned_network_thread_;
    std::unique_ptr<rtc::NetworkManager> owned_network_manager_;
    std::unique_ptr<rtc::PacketSocketFactory> owned_socket_factory_;
};

#endif /* UdpTransport_hpp */
