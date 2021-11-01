#include "packet_switch.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/eventfd.h>
#include <sys/select.h>

#include <vector>

#include "common/log.hpp"

namespace vnet {

packet_switch::packet_switch()
    : fd_tun_(-1)
    , fd_socket_(-1) {
  wakeup_fd_ = eventfd(0, 0);
  if (wakeup_fd_ <= 0) {
    loge() << "failed to create wakeup event";
  }
}

packet_switch::~packet_switch() {
  ::close(wakeup_fd_);
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

int packet_switch::wait() {
  if (socket_to_tun_worker_ && socket_to_tun_worker_->joinable())
    socket_to_tun_worker_->join();

  if (tun_to_socket_worker_ && tun_to_socket_worker_->joinable())
    tun_to_socket_worker_->join();

  logi() << "worker thread exited";
  return 0;
}

void packet_switch::stop() {
  uint64_t n = 0;
  write(wakeup_fd_, &n, sizeof(uint64_t));
}

void packet_switch::forward_data(int in_fd, int out_fd) {
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
    // move data from in fd to pipe
    rc = splice(in_fd, nullptr, fd_pipe[1], nullptr, 4096, SPLICE_F_MOVE | SPLICE_F_MOVE);
    if (rc <= 0) {
      loge() << "failed to move data from in fd to pipe:" << strerror(errno);
      break;
    }

    // move data from pipe to out fd
    rc = splice(fd_pipe[0], nullptr, out_fd, nullptr, rc, SPLICE_F_MOVE | SPLICE_F_MOVE);
    if (rc <= 0) {
      loge() << "failed to move data to pipe to out fd" << strerror(errno);
      break;
    }
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

  // fd set
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(wakeup_fd_, &fds);
  FD_SET(in_fd, &fds);
  while (true) {
    fd_set read_fds;
    memcpy(&read_fds, &fds, sizeof(fds));
    // read data from sourceread_set
    ret = select(in_fd + 1, &read_fds, nullptr, nullptr, nullptr);
    if (ret < 0) {
      loge() << "failed to select in fd:" << strerror(errno);
      return;
    }

    // check and reset wakeup event
    if (FD_ISSET(wakeup_fd_, &read_fds)) {
      // read eventfd to reset the status
      uint64_t n;
      read(wakeup_fd_, &n, sizeof(uint64_t));
      logi() << "wake up event fired, exit worker";
      return;
    }

    // read data
    rlen = ::read(in_fd, buf.data(), buf.size());
    if (rlen <= 0) {
      loge() << "failed to read data from in fd:" << strerror(errno);
      return;
    }

    // write all source data to destination
    left = rlen;
    while (left) {
      wlen = ::write(out_fd, buf.data(), left);
      if (wlen <= 0) {
        loge() << "failed to write data to out fd:" << strerror(errno);
        return;
      }
      left -= wlen;
    }
  }
#endif
}

void packet_switch::forward_tun_to_socket() {
  logi() << "tun -> socket worker is running...";
  forward_data(fd_tun_, fd_socket_);
  logi() << "tun -> socket worker is exiting...";
}

void packet_switch::forward_socket_to_tun() {
  logi() << "socket -> tun worker is running...";
  forward_data(fd_socket_, fd_tun_);
  logi() << "socket -> tun worker is exiting...";
}
} // namespace vnet
