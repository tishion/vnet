#include "tun_iface.hpp"

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#if __linux__
#include <linux/if.h>
#include <linux/if_tun.h>
#elif __APPLE__
#include <net/if.h>
#include <net/if_utun.h>
/* TUNSETIFF ifr flags */
#define TUNSETIFF _IOW('T', 202, int)
#define IFF_TUN 0x0001
#define IFF_TAP 0x0002
#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020
#define IFF_NO_PI 0x1000
#else
#error "Unsupported platform"
#endif
#include <sys/ioctl.h>

#include "common/log.hpp"

namespace vnet {

tun_iface::tun_iface() {
}

tun_iface::~tun_iface() {
  close();
}

int tun_iface::fd() {
  return tun_fd_;
}

bool tun_iface::open() {
  // open the tun device
#if defined(__linux__)
  int fd = ::open("/dev/net/tun", O_RDWR);
#elif defined(__APPLE__)
  int fd = ::open("/dev/tun12", O_RDWR);
#else
#error "unsupported platform"
#endif
  if (fd <= 0) {
    loge() << "failed to open tun device:" << strerror(errno);
    return false;
  }

  // build tun request
  struct ifreq req;
  bzero(&req, sizeof(req));
  req.ifr_flags = IFF_TUN | IFF_NO_PI;

  // send tun device request
  if ((::ioctl(fd, TUNSETIFF, (void*)&req)) < 0) {
    loge() << "failed to set tun flags:" << strerror(errno);
    ::close(fd);
    return false;
  }

  // store fd and device name
  tun_fd_ = fd;
  name_ = req.ifr_name;

  // return
  logi() << "tun device " << name_ << " opened successfully";
  return true;
}

bool tun_iface::set_nonblock(bool nonblock) {
  int flags = ::fcntl(tun_fd_, F_GETFL, 0);
  if (flags < 0) {
    logw() << "failed to get the original flags:" << strerror(errno);
  }

  if (nonblock) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }

  if (::fcntl(tun_fd_, F_SETFL, flags) < 0) {
    loge() << "failed to change the blocking mode:" << strerror(errno);
    return false;
  }

  return true;
}

bool tun_iface::config(const std::string& addr, const std::string& mask) {
  // open a socket to help to set ip address for tun device
  int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    loge() << "failed to open socket to set ip address for tun interface:" << strerror(errno);
    return false;
  }

  // build tun device request
  struct ifreq req;
  bzero(&req, sizeof(req));
  memcpy(&req.ifr_name, name_.data(), name_.length());

  // 1. ****************** set net address ******************
  // build ip address and fill the request
  struct sockaddr_in sock_addr;
  bzero(&sock_addr, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_addr.s_addr = inet_addr(addr.c_str());
  memcpy(&req.ifr_addr, &sock_addr, sizeof(sock_addr));

  // send the tun device request
  if (::ioctl(sock, SIOCSIFADDR, &req) < 0) {
    loge() << "failed to set net address fro interface:" << strerror(errno);
    ::close(sock);
    return false;
  }

  // 2. ****************** set net mask ******************
  struct sockaddr_in sock_mask;
  bzero(&sock_mask, sizeof(sock_mask));
  sock_mask.sin_family = AF_INET;
  sock_mask.sin_addr.s_addr = inet_addr(mask.c_str());
  memcpy(&req.ifr_addr, &sock_mask, sizeof(sock_mask));

  // send the tun device request
  if (::ioctl(sock, SIOCSIFNETMASK, &req) < 0) {
    loge() << "failed to set net mask for interface:" << strerror(errno);
    ::close(sock);
    return false;
  }

  // 3. ****************** bring the interface up******************
  // fill the tun device request with flags
  req.ifr_flags |= IFF_UP;
  req.ifr_flags |= IFF_RUNNING;

  // send the tun device request
  if (::ioctl(sock, SIOCSIFFLAGS, &req) < 0) {
    loge() << "failed to activate intferface: %s\n" << strerror(errno);
    ::close(sock);
    return false;
  }
  // close the helper socket
  ::close(sock);

  // clang-format off
  logi() << "tun device " << name_ << " configuration:\n" 
    << "    net addr:" << addr << "\n"
    << "    net mask:" << mask;
  // clang-format on

  return true;
}

void tun_iface::close() {
  // close tun device
  if (tun_fd_ >= 0) {
    ::close(tun_fd_);
    tun_fd_ = -1;
  }
}
} // namespace vnet
