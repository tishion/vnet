#include "packet_switch_select.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/eventfd.h>
#include <sys/select.h>

#include <vector>

#include "common/log.hpp"
#include "common/utils.h"

namespace vnet {

packet_switch_select::packet_switch_select()
    : fd_tun_(-1)
    , fd_socket_(-1) {
  wakeup_tun_fd_ = eventfd(0, 0);
  if (wakeup_tun_fd_ <= 0) {
    loge() << "failed to create wake up tun event";
  }

  wakeup_udp_fd_ = eventfd(0, 0);
  if (wakeup_udp_fd_ <= 0) {
    loge() << "failed to create wake up udp event";
  }
}

packet_switch_select::~packet_switch_select() {
  ::close(wakeup_tun_fd_);
  ::close(wakeup_udp_fd_);
}

bool packet_switch_select::start(int tun, int socket) {
  fd_tun_ = tun;
  fd_socket_ = socket;

  socket_to_tun_worker_ = std::unique_ptr<std::thread>(
      new std::thread(&packet_switch_select::forward_socket_to_tun, this));

  tun_to_socket_worker_ = std::unique_ptr<std::thread>(
      new std::thread(&packet_switch_select::forward_tun_to_socket, this));

  return true;
}

int packet_switch_select::wait() {
  if (socket_to_tun_worker_ && socket_to_tun_worker_->joinable())
    socket_to_tun_worker_->join();

  if (tun_to_socket_worker_ && tun_to_socket_worker_->joinable())
    tun_to_socket_worker_->join();

  logi() << "worker thread exited";
  return 0;
}

void packet_switch_select::stop() {
  logi() << "send wake up signal...";
  uint64_t n = 1;
  if (write(wakeup_udp_fd_, &n, sizeof(uint64_t)) <= 0) {
    loge() << "failed to send wake up udp signal";
  }
  if (write(wakeup_tun_fd_, &n, sizeof(uint64_t)) <= 0) {
    loge() << "failed to send wake up tun signal";
  }
}

void packet_switch_select::forward_data(int in_fd, int out_fd, int wakeup_fd) {
// currently the linux kernel build-in tun driver doesn't support
// splice operation, so we cannot leverage the zero copy technology
// for better performance to redirect the data, refer to :
// https://github.com/torvalds/linux/blob/master/drivers/net/tun.c
// https://core.ac.uk/download/pdf/44404755.pdf
// https://linuxgazette.net/149/misc/melinte/
// if performace is critically required then we need to implement
// a custom tun device dirver.
#if defined(ZERO_COPY_TRANSFER)
  int fd_pipe[2];
  int rc = pipe(fd_pipe);

  while (true) {
    forward_with_splice(in_fd, out_fd, fd_pipe);
  }

  // close pipe
  close(fd_pipe[0]);
  close(fd_pipe[1]);
#else
  // paratmers
  int rlen = 0;
  int wlen = 0;
  int left = 0;
  int ret = 0;

  // buf
  std::vector<uint8_t> buf(4096);
  while (true) {
    // prepare fd set
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(wakeup_fd, &read_fds);
    FD_SET(in_fd, &read_fds);

    // read data from sourceread_set
    ret = select(FD_SETSIZE, &read_fds, nullptr, nullptr, nullptr);
    if (ret < 0) {
      loge() << "failed to select in fd:" << strerror(errno);
      return;
    }

    // check and reset wakeup event
    if (FD_ISSET(wakeup_fd, &read_fds)) {
      logv() << "wake up signal fired";
      // read eventfd to reset the status
      uint64_t n;
      read(wakeup_fd, &n, sizeof(uint64_t));
      logi() << "wake up event fired, exit worker";
      return;
    }

    if (FD_ISSET(in_fd, &read_fds)) {
      forward_with_rw(in_fd, out_fd);
    }
  }
#endif
}

void packet_switch_select::forward_tun_to_socket() {
  logi() << "tun -> socket worker is running...";
  forward_data(fd_tun_, fd_socket_, wakeup_tun_fd_);
  logi() << "tun -> socket worker is exiting...";
}

void packet_switch_select::forward_socket_to_tun() {
  logi() << "socket -> tun worker is running...";
  forward_data(fd_socket_, fd_tun_, wakeup_udp_fd_);
  logi() << "socket -> tun worker is exiting...";
}
} // namespace vnet
