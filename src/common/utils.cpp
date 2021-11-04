#include "utils.h"

#include <cstdint>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "log.hpp"

int forward_with_rw(int in_fd, int out_fd) {
  // paratmers
  int rlen = 0;
  int wlen = 0;
  int left = 0;

  // read data from source
  std::vector<uint8_t> buf(4096);
  rlen = ::read(in_fd, buf.data(), buf.size());
  if (rlen <= 0) {
    loge() << "failed to read data from in fd:" << strerror(errno);
    return -1;
  }

  logv() << "<<<< read " << rlen << " bytes from in fd";

  // write all source data to destination
  left = rlen;
  while (left) {
    wlen = ::write(out_fd, buf.data(), left);
    if (wlen <= 0) {
      loge() << "failed to write data to out fd:" << strerror(errno);
      return (rlen - left);
    }
    logv() << ">>>> write " << wlen << " bytes to out fd";

    left -= wlen;
  }

  return wlen;
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
