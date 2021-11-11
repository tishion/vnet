#include "utils.h"

#include <cstdint>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

#include "log.hpp"

int forward_with_readwrite(uint8_t* buf, int size, int in_fd, int out_fd) {
  int transferred = 0;

  // paratmers
  int rlen = 0;
  while (true) {
    rlen = ::read(in_fd, buf, size);
    if (rlen <= 0) {
      if (errno != EAGAIN) {
        loge() << "failed to read data from in fd" << strerror(errno);
      }
      break;
    }

    logv() << "<<<< read " << rlen << " bytes from in fd";

    // write all source data to destination
    int offset = 0;
    int wlen = 0;
    int left = rlen;
    while (left) {
      wlen = ::write(out_fd, buf + offset, left);
      if (wlen <= 0 && errno != EAGAIN) {
        if (errno == EAGAIN) {
          continue;
        } else {
          loge() << "failed to write data to out fd:" << strerror(errno);
          return transferred;
        }
      }
      logv() << ">>>> write " << wlen << " bytes to out fd";

      left -= wlen;
      offset += wlen;
      transferred += wlen;
    }
  }

  return transferred;
}

int forward_with_splice(int in_fd, int out_fd, int pipe[2]) {
  int rc = 0;
  // move data from in fd to pipe
  rc = splice(in_fd, nullptr, pipe[1], nullptr, 4096, SPLICE_F_MOVE | SPLICE_F_MOVE);
  if (rc <= 0) {
    loge() << "failed to move data from in fd to pipe:" << strerror(errno);
    return -1;
  }

  // move data from pipe to out fd
  rc = splice(pipe[0], nullptr, out_fd, nullptr, rc, SPLICE_F_MOVE | SPLICE_F_MOVE);
  if (rc <= 0) {
    loge() << "failed to move data to pipe to out fd" << strerror(errno);
    return -1;
  }

  return rc;
}
