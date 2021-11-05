#include "application.hpp"

#include "common/log.hpp"

namespace vnet {

application::application() {
}

application::~application() {
}

int application::run(const std::string& tun_ip, const std::string& remote_ip, uint16_t vn_port) {
  if (!udp_socket_.open()) {
    loge() << "failed to create socket for lower connection";
    return -1;
  }

  if (!udp_socket_.bind("0.0.0.0", vn_port)) {
    loge() << "failed to bind connection socket to local address";
    return -1;
  }

  if (!udp_socket_.connect(remote_ip, vn_port)) {
    loge() << "failed to connect to remote peer";
    return -1;
  }

  if (!tun_iface_.open()) {
    loge() << "failed to open tun device";
    return -1;
  }

  if (!tun_iface_.config(tun_ip, "255.255.255.0")) {
    loge() << "failed to configure tun device";
    return -1;
  }

  // start packet switch
  packet_switch_.start(tun_iface_.fd(), udp_socket_.fd());
  int rc = packet_switch_.wait();

  // close tun device
  tun_iface_.close();

  // close lower socket
  udp_socket_.close();

  // return error code
  return rc;
}

void application::stop() {
  packet_switch_.stop();
}
} // namespace vnet
