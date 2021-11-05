#ifndef PACKET_SWITCH_H
#define PACKET_SWITCH_H
#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "packet_switch_epoll.hpp"
#include "packet_switch_select.hpp"

namespace vnet {

/**
 * @brief
 *
 */
class packet_switch {
public:
  packet_switch();
  ~packet_switch();

  bool start(int tun, int socket);
  int wait();
  void stop();

private:
  packet_switch_epoll switch_;
};
} // namespace vnet

#endif