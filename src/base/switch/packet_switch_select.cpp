#include "packet_switch_select.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/eventfd.h>
#include <sys/select.h>

#include <array>

#include "common/log.hpp"
#include "common/utils.h"

#define max(a, b)                                                                                  \
  ({                                                                                               \
    __typeof__(a) _a = (a);                                                                        \
    __typeof__(b) _b = (b);                                                                        \
    _a > _b ? _a : _b;                                                                             \
  })

namespace vnet {

packet_switch_select::packet_switch_select()
    : fd_tun_(-1)
    , fd_socket_(-1) {
  wakeup_fd_ = eventfd(0, 0);
  if (wakeup_fd_ <= 0) {
    loge() << "failed to create wake up tun event";
  }
}

packet_switch_select::~packet_switch_select() {
  ::close(wakeup_fd_);
}

bool packet_switch_select::start(int tun, int socket) {
  fd_tun_ = tun;
  fd_socket_ = socket;

  worker_thread_ =
      std::unique_ptr<std::thread>(new std::thread(&packet_switch_select::process, this));

  return true;
}

int packet_switch_select::wait() {
  if (worker_thread_ && worker_thread_->joinable())
    worker_thread_->join();

  logi() << "worker thread exited";
  return 0;
}

void packet_switch_select::stop() {
  logi() << "send wake up signal...";
  exit_ = true;
  uint64_t n = 1;
  if (write(wakeup_fd_, &n, sizeof(uint64_t)) <= 0) {
    loge() << "failed to send wake up udp signal";
  }
}

void packet_switch_select::process() {
  logi() << "+++++ process worker is running...";

  int in_fd = 0;
  int out_fd = 0;

#if defined(SPLICE_TRANSFER)
  // pipe
  int fd_pipe[2];
  int rc = pipe(fd_pipe);
  int in_flags = 0;
  int out_flags = 0;
#else
  // buf
  std::array<uint8_t, 16 * 1024> buf;
#endif

  // entering loop
  exit_ = false;
  while (exit_) {
    // prepare fd set
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(wakeup_fd_, &read_fds);
    FD_SET(fd_tun_, &read_fds);
    FD_SET(fd_socket_, &read_fds);

    int fd_limit = max(max(fd_tun_, fd_socket_), wakeup_fd_) + 1;
    int ret = select(fd_limit, &read_fds, nullptr, nullptr, nullptr);
    if (ret < 0) {
      loge() << "failed to select fds:" << strerror(errno);
      return;
    }

    // check result
    for (int fd = 0; fd < fd_limit; fd++) {
      if (FD_ISSET(fd, &read_fds)) {

        if (fd == fd_tun_) {
          in_fd = fd_tun_;
          out_fd = fd_socket_;
#if defined(SPLICE_TRANSFER)
          in_flags = SPLICE_F_MOVE;
          out_flags = SPLICE_F_MOVE | SPLICE_F_MORE;
#endif
        } else if (fd == fd_socket_) {
          in_fd = fd_socket_;
          out_fd = fd_tun_;
#if defined(SPLICE_TRANSFER)
          in_flags = SPLICE_F_MOVE;
          out_flags = SPLICE_F_MOVE;
#endif
        } else {
          // clear wakeup fd state
          if (fd == wakeup_fd_) {
            logv() << "wake up signal fired";
            // read eventfd to reset the status
            uint64_t n;
            read(wakeup_fd_, &n, sizeof(uint64_t));
            logi() << "wake up event fired, exit worker";
          } else {
            loge() << "unknown fd";
          }
          continue;
        }

        // forward data
#if defined(SPLICE_TRANSFER)
        forward_with_splice(fd_pipe, in_fd, in_flags, out_fd, out_flags);
#else
        forward_with_readwrite(buf.data(), buf.size(), in_fd, out_fd);
#endif
      }
    }
  }

#if defined(SPLICE_TRANSFER)
  // close pipe
  close(fd_pipe[0]);
  close(fd_pipe[1]);
#endif

  logi() << "----- process worker is exiting...";
}
} // namespace vnet
