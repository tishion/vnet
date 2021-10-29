#include "packet_switch.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <vector>

#include "common/log.hpp"

namespace vnet {

packet_switch::packet_switch()
    : fd_tun_(-1)
    , fd_socket_(-1) {
}

packet_switch::~packet_switch() {
}

bool packet_switch::start(int tun, int socket) {
  fd_tun_ = tun;
  fd_socket_ = socket;

  socket_to_tun_worker_ =
      std::make_unique<std::thread>(&packet_switch::forward_socket_to_tun, this);

  tun_to_socket_worker_ =
      std::make_unique<std::thread>(&packet_switch::forward_tun_to_socket, this);

  return true;
}

void packet_switch::stop() {
}

int packet_switch::wait() {
  if (socket_to_tun_worker_ && socket_to_tun_worker_->joinable())
    socket_to_tun_worker_->join();

  if (tun_to_socket_worker_ && tun_to_socket_worker_->joinable())
    tun_to_socket_worker_->join();

  return 0;
}

void packet_switch::forward_data(int in_fd, int out_fd, unsigned int out_flags) {

#if defined(ZERO_COPY_TRANSFER)
  int fd_pipe[2];
  int rc = pipe(fd_pipe);

  while (true) {
    // move data from in fd to pipe
    rc = splice(in_fd, nullptr, fd_pipe[1], nullptr, 4096, 0);
    if (rc <= 0) {
      loge() << "failed to move data from in fd to pipe:" << strerror(errno);
      break;
    }

    // move data from pipe to out fd
    rc = splice(fd_pipe[0], nullptr, out_fd, nullptr, rc, out_flags);
    if (rc <= 0) {
      loge() << "failed to move data to pipe to out fd" << strerror(errno);
      break;
    }
  }

  // close pipe
  close(fd_pipe[0]);
  close(fd_pipe[1]);
#else
  int rlen = 0;
  int wlen = 0;
  int left = 0;
  std::vector<uint8_t> buf(4096);
  while (true) {
    rlen = ::read(in_fd, buf.data(), buf.size());
    if (rlen < 0) {
      loge() << "failed to read data from in fd:" << strerror(errno);
      break;
    }

    left = rlen;
    while (left) {
      wlen = ::write(out_fd, buf.data(), left);
      if (wlen < 0) {
        loge() << "failed to write data to out fd:" << strerror(errno);
        break;
      }
      left -= wlen;
    }
  }
#endif
}

void packet_switch::forward_tun_to_socket() {
  forward_data(fd_tun_, fd_socket_, SPLICE_F_MOVE | SPLICE_F_MOVE);
}

void packet_switch::forward_socket_to_tun() {
  forward_data(fd_socket_, fd_tun_, SPLICE_F_MOVE);
}
} // namespace vnet