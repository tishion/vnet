#include "log.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#elif defined(_WIN32)
#include <processthreadsapi.h>
#endif

#if defined(__APPLE__)
#include <pthread.h>
#endif

// There is no wrapper function gettid() in glibc below 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

namespace common {

namespace details {

/**
 * @brief
 *
 * @return std::string
 */
inline std::string FormatTimestamp() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
  auto t = system_clock::to_time_t(now);
  auto lt = std::localtime(&t);
  char ts[sizeof("YYYY-mm-ddTHH:MM:SS+0000")];
  strftime(ts, sizeof(ts), "%FT%T%z", lt);

  std::ostringstream oss;
  oss << ts << "-" << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}

inline std::string FormatLogLevel(logging_level level) {
  // clang-format off
  static const char* sLeveLTable[] = {
    "-",
    "V", 
    "D", 
    "I", 
    "W", 
    "E"
  };
  // clang-format on
  if (level <= 0 || level >= LS_MAXLEVEL)
    level = LS_UNKNOWN;

  return sLeveLTable[level];
}

inline std::string FormatProcessId() {
  uint32_t pid = 0;
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
  pid = static_cast<uint32_t>(getpid());
#else
  pid = static_cast<uint32_t>(GetProcessId(NULL));
#endif
  return std::to_string(pid);
}

inline std::string FormatThreadId() {
  uint64_t tid = 0;
#if defined(__linux__) || defined(__unix__)
  tid = static_cast<uint64_t>(gettid());
#elif defined(__APPLE__)
  pthread_threadid_np(NULL, &tid);
  tid = static_cast<uint64_t>(tid);
#endif
  return std::to_string(tid);
}

} // namespace details

logger::message::message(logging_level lvl)
    : level_(lvl) {
  // clang-format off
  *this << details::FormatTimestamp() << "|" 
      << details::FormatProcessId() << ":"
      << details::FormatThreadId() << "|" 
      << details::FormatLogLevel(level_) << "|";
  // clang-format on
}

logger& logger::default_instance() {
  static logger s_instance;
  return s_instance;
}

logger::logger()
    : level_(LS_VERBOSE) {
}

logger::~logger() {
}

void logger::operator&=(message& msg) {
  if (msg.level_ >= this->level_) {
    std::cout << msg.oss_.str() << std::endl;
  }
}
} // namespace common
