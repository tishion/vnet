#ifndef PACKET_SWITCH_SELECT_H
#define PACKET_SWITCH_SELECT_H
#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace vnet {

/**
 * @brief
 *
 */
class packet_switch_select {
public:
  packet_switch_select();
  ~packet_switch_select();

  bool start(int tun, int socket);
  int wait();
  void stop();

protected:
  void process();

private:
  int fd_tun_;
  int fd_socket_;
  int wakeup_fd_;

  bool exit_ = false;
  std::unique_ptr<std::thread> worker_thread_;
};
} // namespace vnet

#endif