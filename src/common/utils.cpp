#include "utils.h"

#include <cstdint>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
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
        loge() << "failed to read data from in fd: " << strerror(errno);
      }
      break;
    }

    logv() << "<<<< read " << rlen << " bytes from in fd";

    // move all data from int to out
    int offset = 0;
    int wlen = 0;
    int left = rlen;
    while (left) {
      wlen = ::write(out_fd, buf + offset, left);
      if (wlen <= 0) {
        if (errno == EAGAIN) {
          continue;
        } else {
          loge() << "failed to write data to out fd: " << strerror(errno);
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

int forward_with_splice(int pipe[2], int in_fd, int in_flags, int out_fd, int out_flags) {
  int transferred = 0;

  // paratmers
  int rlen = 0;
  while (true) {
    rlen = splice(in_fd, nullptr, pipe[1], nullptr, 16 * 1024, in_flags);
    if (rlen <= 0) {
      if (errno != EAGAIN) {
        loge() << "failed to move data from in fd to pipe: " << strerror(errno);
      }
      break;
    }

    logv() << "<<<< moved " << rlen << " bytes from in fd to pipe";

    int wlen = 0;
    int left = rlen;
    while (left) {
      // move all data from int to out
      wlen = splice(pipe[0], nullptr, out_fd, nullptr, left, out_flags);
      if (wlen <= 0) {
        if (errno == EAGAIN) {
          continue;
        } else {
          loge() << "failed to move data from pipe to out fd: " << strerror(errno)
                 << ", transferred: " << transferred;
          return transferred;
        }
      }

      logv() << ">>>> moved " << wlen << " bytes from pipe to out fd";

      left -= wlen;
      transferred += wlen;
    }
  }

  return transferred;
}

// int forward_with_splice(int pipe[2], int in_fd, int in_flags, int out_fd, int out_flags) {
//   int rc = 0;
//   // move data from in fd to pipe
//   rc = splice(in_fd, nullptr, pipe[1], nullptr, 4096, in_flags);
//   if (rc <= 0) {
//     loge() << "failed to move data from in fd to pipe: " << strerror(errno);
//     return -1;
//   }

//   logv() << "<<<< moved " << rc << " bytes from in fd to pipe";

//   // move data from pipe to out fd
//   rc = splice(pipe[0], nullptr, out_fd, nullptr, rc, out_flags);
//   if (rc <= 0) {
//     loge() << "failed to move data from pipe to out fd: " << strerror(errno);
//     return -1;
//   }

//   logv() << ">>>> moved " << rc << " bytes from pipe to out fd";

//   return rc;
// }
