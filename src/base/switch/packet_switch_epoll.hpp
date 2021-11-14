#ifndef PACKET_SWITCH_EPOLL_H
#define PACKET_SWITCH_EPOLL_H
#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "common/evtio.hpp"

namespace vnet {

/**
 * @brief
 *
 */
class packet_switch_epoll {
public:
  packet_switch_epoll();
  ~packet_switch_epoll();

  bool start(int tun, int socket);
  int wait();
  void stop();

protected:
  void process();

private:
  int fd_tun_;
  int fd_socket_;

  evtio::evt evt_;
  bool exit_ = false;
  std::unique_ptr<std::thread> worker_thread_;
};
} // namespace vnet

#endif
