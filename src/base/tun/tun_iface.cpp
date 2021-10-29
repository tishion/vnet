#include "tun_iface.hpp"

#include <errno.h>
#include <fcntl.h>
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
#define TUNSETIFF       _IOW('T', 202, int) 
#define IFF_TUN         0x0001
#define IFF_TAP         0x0002
#define IFF_NAPI        0x0010
#define IFF_NAPI_FRAGS  0x0020
#define IFF_NO_PI       0x1000
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
  int fd = ::open("/dev/net/tun", O_RDWR);
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
  logi() << "TUN device opened:" << name_;
  return true;
}

bool tun_iface::set_ip(const std::string& ip) {
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

  // build ip address and fill the request
  struct sockaddr_in sock_addr;
  bzero(&sock_addr, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_addr.s_addr = inet_addr(ip.c_str());
  memcpy(&req.ifr_addr, &sock_addr, sizeof(sock_addr));

  // send the tun device request
  if (::ioctl(sock, SIOCSIFADDR, &req) < 0) {
    loge() << "failed to set SIOCSIFADDR:" << strerror(errno);
    ::close(sock);
    return false;
  }

  // fill the tun device request with flags
  req.ifr_flags |= IFF_UP;
  req.ifr_flags |= IFF_RUNNING;

  // send the tun device request
  if (::ioctl(sock, SIOCSIFFLAGS, &req) < 0) {
    loge() << "failed to set SIOCSIFFLAGS: %s\n" << strerror(errno);
    ::close(sock);
    return false;
  }
  // close the helper socket
  ::close(sock);

  // return
  logi() << "TUN device was bound to ip:" << ip;
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
