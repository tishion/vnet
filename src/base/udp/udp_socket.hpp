#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H
#pragma once

#include <cstdint>
#include <string>

namespace vnet {

/**
 * @brief
 *
 */
class udp_socket {
public:
  /**
   * @brief Construct a new udp socket object
   *
   */
  udp_socket();

  /**
   * @brief Destroy the udp socket object
   *
   */
  ~udp_socket();

  /**
   * @brief
   *
   * @return int
   */
  int fd();

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool open();

  /**
   * @brief
   *
   * @param ip
   * @param port
   * @return true
   * @return false
   */
  bool bind(const std::string& ip, uint16_t port);

  /**
   * @brief
   *
   * @param ip
   * @param port
   * @return true
   * @return false
   */
  bool connect(const std::string& ip, uint16_t port);

  /**
   * @brief
   *
   */
  void close();

private:
  /**
   * @brief
   *
   */
  int sock_fd_ = -1;
};
} // namespace vnet
#endif