#include "packet_switch.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <vector>

#include "common/log.hpp"

namespace vnet {

packet_switch::packet_switch() {
}

packet_switch::~packet_switch() {
}

bool packet_switch::start(int tun, int socket) {
  return switch_.start(tun, socket);
}

int packet_switch::wait() {
  return switch_.wait();
}

void packet_switch::stop() {
  switch_.stop();
}
} // namespace vnet
