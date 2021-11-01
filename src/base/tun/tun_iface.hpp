#ifndef TUN_IFACE_H
#define TUN_IFACE_H
#pragma once

#include <cstdint>
#include <string>

namespace vnet {

/**
 * @brief
 *
 */
class tun_iface {
public:
  /**
   * @brief Construct a new tun iface object
   *
   */
  tun_iface();

  /**
   * @brief Destroy the tun iface object
   *
   */
  ~tun_iface();

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
   * @brief Set the ip object
   *
   * @param ip
   * @return true
   * @return false
   */
  bool config(const std::string& addr, const std::string& mask);

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
  int tun_fd_ = -1;

  /**
   * @brief
   *
   */
  std::string name_;
};
} // namespace vnet

#endif