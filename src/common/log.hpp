#ifndef LOG_H
#define LOG_H
#pragma once

#include <iostream>
#include <sstream>

namespace common {

enum logging_level
{
  LS_UNKNOWN,
  LS_VERBOSE,
  LS_DEBUG,
  LS_INFO,
  LS_WARNING,
  LS_ERROR,

  // MAX
  LS_MAXLEVEL
};

class logger {
public:
  class message {
  public:
    message(logging_level lvl);

    template <class T>
    message& operator<<(const T& msg) {
      oss_ << msg;
      return *this;
    }

    logging_level level_;
    std::ostringstream oss_;
  };

public:
  static logger& default_instance();

  logger();
  ~logger();

  void operator&=(message& msg);

private:
  logging_level level_;
};
} // namespace common

#define log(level) common::logger::default_instance() &= common::logger::message(level)

#define logv() log(common::LS_VERBOSE)
#define logd() log(common::LS_DEBUG)
#define logi() log(common::LS_INFO)
#define logw() log(common::LS_WARNING)
#define loge() log(common::LS_ERROR)

#endif