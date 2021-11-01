#ifndef APPLICATION_H
#define APPLICATION_H
#pragma once

#include <string>

#include "base/packet_switch.hpp"
#include "base/tun/tun_iface.hpp"
#include "base/udp/udp_socket.hpp"

namespace vnet {

class application {
private:
public:
  application();
  ~application();

  int run(const std::string& tun_ip, const std::string& remote_ip, uint16_t remote_port);

  void stop();

private:
  udp_socket udp_socket_;

  tun_iface tun_iface_;

  packet_switch packet_switch_;
};
} // namespace vnet

#endif