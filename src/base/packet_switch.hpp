#ifndef PACKET_SWITCH_H
#define PACKET_SWITCH_H
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
class packet_switch {
public:
  packet_switch();
  ~packet_switch();

  bool start(int tun, int socket);
  void stop();
  int wait();

protected:
  void forward_data(int in_fd, int out_fd);
  void forward_tun_to_socket();
  void forward_socket_to_tun();

private:
  int fd_tun_;
  int fd_socket_;

  std::unique_ptr<std::thread> tun_to_socket_worker_;
  std::unique_ptr<std::thread> socket_to_tun_worker_;

  std::mutex mtx_;
  std::condition_variable stop_;
};
} // namespace vnet

#endif