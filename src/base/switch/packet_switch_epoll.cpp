#include "packet_switch_epoll.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>

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

  if (!evt_.open()) {
    loge() << "failed to open evtio";
    return;
  }

  evtio::evt_context tun_ctx_(fd_tun_, nullptr);
  evtio::evt_context udp_ctx(fd_socket_, nullptr);
  if (!evt_.attach(&tun_ctx_, evtio::op_read)) {
    loge() << "failed to add tun fd to evtio";
    return;
  }

  if (!evt_.attach(&udp_ctx, evtio::op_read)) {
    evt_.detach(&tun_ctx_);
    loge() << "failed to add udp fd to evtio";
    return;
  }

  exit_ = false;

  evtio::evt_event_list events;
  while (!exit_) {
    bool ok = evt_.wait(events, 2, -1);
    if (!ok) {
      logw() << "faild to wait";
    }

    for (auto e : events) {
      if (e.context->handle == fd_tun_) {
        forward_with_rw(fd_tun_, fd_socket_);
      } else if (e.context->handle == fd_socket_) {
        forward_with_rw(fd_socket_, fd_tun_);
      } else {
        loge() << "unknown event";
      }
    }
  }

  evt_.close();

  logi() << "----- process worker is exiting...";
}
} // namespace vnet
