#include "packet_switch_epoll.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>

#include <array>

#if defined(__APPLE__)
int eventfd(int a, int b) {
  return 0;
}
#elif defined(__linux__)
#include <sys/eventfd.h>
#endif

#include <vector>

#include "common/log.hpp"
#include "common/utils.h"

namespace vnet {

packet_switch_epoll::packet_switch_epoll()
    : fd_tun_(-1)
    , fd_socket_(-1) {
}

packet_switch_epoll::~packet_switch_epoll() {
}

bool packet_switch_epoll::start(int tun, int socket) {
  fd_tun_ = tun;
  fd_socket_ = socket;

  worker_thread_ =
      std::unique_ptr<std::thread>(new std::thread(&packet_switch_epoll::process, this));

  return true;
}

int packet_switch_epoll::wait() {
  if (worker_thread_ && worker_thread_->joinable())
    worker_thread_->join();

  logi() << "worker thread exited";
  return 0;
}

void packet_switch_epoll::stop() {
  logi() << "send wake up signal...";
  exit_ = true;
  evt_.wakeup();
}

void packet_switch_epoll::process() {
  logi() << "+++++ process worker is running...";

  int in_fd = 0;
  int out_fd = 0;
  evtio::evt_event_list events;

  if (!evt_.open()) {
    loge() << "failed to open evtio";
    return;
  }

  evtio::evt_context tun_ctx_(fd_tun_, nullptr);
  if (!evt_.attach(&tun_ctx_, evtio::op_read)) {
    loge() << "failed to add tun fd to evtio";
    return;
  }

  evtio::evt_context udp_ctx(fd_socket_, nullptr);
  if (!evt_.attach(&udp_ctx, evtio::op_read)) {
    evt_.detach(&tun_ctx_);
    loge() << "failed to add udp fd to evtio";
    return;
  }

#if defined(SPLICE_TRANSFER)
  int in_flags = 0;
  int out_flags = 0;
  int* fd_pipe = 0;
  int fd_tun_sock_pipe[2];
  pipe(fd_tun_sock_pipe);
  int fd_sock_tun_pipe[2];
  pipe(fd_sock_tun_pipe);
#else
  std::array<uint8_t, 16 * 1024> buf;
#endif

  // entering loop
  exit_ = false;
  while (!exit_) {
    bool ok = evt_.wait(events, 2, -1);
    if (!ok) {
      logw() << "failed to wait";
      return;
    }

    // check result
    for (auto e : events) {
      if (e.context->handle == fd_tun_) {
        logv() << "read tun >>>>>>>>>>>>> write socket";
        in_fd = fd_tun_;
        out_fd = fd_socket_;
#if defined(SPLICE_TRANSFER)
        fd_pipe = fd_tun_sock_pipe;
        in_flags = SPLICE_F_MOVE;
        out_flags = SPLICE_F_MOVE;
#endif
      } else if (e.context->handle == fd_socket_) {
        logv() << "read socket >>>>>>>>>>>>> write tun";
        in_fd = fd_socket_;
        out_fd = fd_tun_;
#if defined(SPLICE_TRANSFER)
        fd_pipe = fd_sock_tun_pipe;
        in_flags = SPLICE_F_MOVE;
        out_flags = SPLICE_F_MOVE;
#endif
      } else {
        loge() << "unknown event";
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

#if defined(SPLICE_TRANSFER)
  // close pipe
  close(fd_tun_sock_pipe[0]);
  close(fd_tun_sock_pipe[1]);
  close(fd_sock_tun_pipe[0]);
  close(fd_sock_tun_pipe[1]);
#endif

  evt_.close();

  logi() << "----- process worker is exiting...";
}
} // namespace vnet
