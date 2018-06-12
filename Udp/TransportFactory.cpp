//
//  UdpTransport.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/1.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "TransportFactory.h"
#include "api/proxy.h"
#include "ortc/ortcfactory.h"

TransportFactory* TransportFactory::Instance() {
    RTC_DEFINE_STATIC_LOCAL(TransportFactory, ls_transport_factory, ());
    return &ls_transport_factory;
}

TransportFactory::TransportFactory(){
    //if (!network_thread_)
    {
        owned_network_thread_ = rtc::Thread::CreateWithSocketServer();
        owned_network_thread_->Start();
        network_thread_ = owned_network_thread_.get();
    }
    
    //if (!network_manager_)
    {
        owned_network_manager_.reset(new rtc::BasicNetworkManager());
        network_manager_ = owned_network_manager_.get();
    }
    
    //if (!socket_factory_)
    {
        owned_socket_factory_.reset(new rtc::BasicPacketSocketFactory(network_thread_));
        socket_factory_ = owned_socket_factory_.get();
    }
}

std::unique_ptr<UdpTransportProxy> TransportFactory::CreateUdpTransport(std::string name, bool ipv6){
    webrtc::MethodCall2<TransportFactory, std::unique_ptr<UdpTransportProxy>, std::string, bool> call(this, &TransportFactory::CreateUdpTransport_w, std::move(name), ipv6);
    return call.Marshal(RTC_FROM_HERE, network_thread_);
}

std::unique_ptr<UdpTransportProxy> TransportFactory::CreateUdpTransport_w(std::string tch_name, bool ipv6){
    std::unique_ptr<rtc::AsyncPacketSocket> socket(socket_factory_->CreateUdpSocket(rtc::SocketAddress(rtc::GetAnyIP(ipv6?AF_INET6:AF_INET), 0),0,0));
    
    return std::unique_ptr<UdpTransportProxy>(new UdpTransportProxy(network_thread_, new cricket::UdpTransport(std::move(tch_name), std::move(socket))));
}
