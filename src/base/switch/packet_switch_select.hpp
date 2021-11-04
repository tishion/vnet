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
  void forward_data(int in_fd, int out_fd, int wakeup_fd);
  void forward_tun_to_socket();
  void forward_socket_to_tun();

private:
  int wakeup_tun_fd_;
  int wakeup_udp_fd_;

  int fd_tun_;
  int fd_socket_;

  std::unique_ptr<std::thread> tun_to_socket_worker_;
  std::unique_ptr<std::thread> socket_to_tun_worker_;
};
} // namespace vnet

#endif