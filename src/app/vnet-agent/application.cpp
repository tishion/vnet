#include "application.hpp"

#include "common/log.hpp"

#include "base/tun/tun_iface.hpp"
#include "base/udp/udp_socket.hpp"

namespace vnet {

application::application() {
}

application::~application() {
}

int application::run(const std::string& tun_ip, const std::string& remote_ip,
                     uint16_t remote_port) {
  udp_socket udp_socket;
  if (!udp_socket.open()) {
    loge() << "failed to create socket for lower connection";
    return -1;
  }

  if (!udp_socket.bind("0.0.0.0", 8888)) {
    loge() << "failed to bind connection socket to local address";
    return -1;
  }

  if (!udp_socket.connect(remote_ip, remote_port)) {
    loge() << "failed to connect to remote peer";
    return -1;
  }

  tun_iface tun_iface;
  if (!tun_iface.open()) {
    loge() << "failed to open tun device";
    return -1;
  }

  if (!tun_iface.set_ip(tun_ip)) {
    loge() << "failed to set ip for tun device";
    return -1;
  }

  // start packet switch
  packet_switch_.start(tun_iface.fd(), udp_socket.fd());
  int rc = packet_switch_.wait();

  // close tun device
  tun_iface.close();

  // close lower socket
  udp_socket.close();

  // return error code
  return rc;
}

void application::stop() {
}
} // namespace vnet