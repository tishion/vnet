#include <signal.h>
#include <string.h>

#include <arpa/inet.h>

#include "common/log.hpp"

#include "application.hpp"

vnet::application app_;

static void sigexit(int signo) {
  logi() << "exit action received...";
  app_.stop();
}

static void set_signal(int signo, void (*handler)(int)) {
  struct sigaction sa;
  bzero(&sa, sizeof(sa));

  sa.sa_handler = (void (*)(int))handler;
  // #ifdef SA_INTERRUPT
  //   sa.sa_flags = SA_INTERRUPT;
  // #endif
  if (sigaction(signo, &sa, nullptr) < 0) {
    loge() << "failed to set signal handler";
  }
}

static void usage(const char* program) {
  logi() << "Usage:" << program << " vn-port tun-ip remote-ip";
  logi() << "Example:" << program << "8888 192.168.12.34 10.10.10.10";
}

int main(int argc, char** argv) {
  if (argc != 4) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  // extract args
  uint16_t vn_port = std::atoi(argv[1]);
  std::string tun_ip = argv[2];
  std::string remote_ip = argv[3];

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
  return app_.run(tun_ip, remote_ip, vn_port);
}
