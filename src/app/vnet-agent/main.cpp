#include <arpa/inet.h>
#include <string.h>
#include <sys/signal.h>

#include "common/log.hpp"

#include "base/packet_switch.hpp"
#include "base/tun/tun_iface.hpp"
#include "base/udp/udp_socket.hpp"

#include "application.hpp"

vnet::application app_;

static void sigexit(int signo) {
  app_.stop();
}

static void set_signal(int signo, void (*handler)(int)) {
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));

  sa.sa_handler = (void (*)(int))handler;
#ifdef SA_INTERRUPT
  sa.sa_flags = SA_INTERRUPT;
#endif
  sigaction(signo, &sa, NULL);
}

static void usage(const char* program) {
  logi() << "Usage:" << program << " tun-ip remote-ip remote-port";
  logi() << "Example:" << program << " 172.16.0.1  10.10.10.10 7000";
}

int main(int argc, char** argv) {
  if (argc != 4) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  // extract args
  std::string tun_ip = argv[1];
  std::string remote_ip = argv[2];
  uint16_t remote_port = std::atoi(argv[3]);

  uint32_t tmp = 0;

  // validatet tun ip address
  if (0 >= inet_pton(AF_INET, tun_ip.c_str(), &tmp)) {
    logi() << "invalid IP address:" << tun_ip;
    exit(EXIT_FAILURE);
  }

  // validate remote ip address
  if (0 >= inet_pton(AF_INET, remote_ip.c_str(), &tmp)) {
    logi() << "invalid IP address:" << remote_ip;
    exit(EXIT_FAILURE);
  }

  // set signal handler
  set_signal(SIGINT, sigexit);
  set_signal(SIGQUIT, sigexit);

  // start the application
  return app_.run(tun_ip, remote_ip, remote_port);
}
