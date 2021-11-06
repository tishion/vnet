#ifndef EVTIO_H
#define EVTIO_H
#pragma once

#pragma region stl header
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#pragma endregion

#pragma region platform header
#include <errno.h>
#include <string.h>
#include <unistd.h>
#pragma endregion

#if defined(__ANDROID__)
#define ANDROID

typedef int evt_handle;
#elif defined(__linux__)
#define LINUX
#include <sys/epoll.h>
#include <sys/eventfd.h>

typedef int evt_handle;

#elif defined(__APPLE__)
#define APPLE
#include <sys/event.h>

typedef int evt_handle;

#elif defined(_WIN32)
#define WINDOWS
#include <windows.h>

typedef HANDLE evt_handle;

#endif

#if defined(EVTIO_ENABLE_LOG)
#define logE() logger::instance() &= logger::message("ERR")
#define logI() logger::instance() &= logger::message("INF")
#else
#define logE() std::ostringstream()
#define logI() std::ostringstream()
#endif

namespace evtio {
/**
 * @brief
 *
 */
enum evt_operation {
  /**
   * @brief
   */
  op_read = 0x01,

  /**
   * @brief
   */
  op_write = 0x02,

  /**
   * @brief
   */
  OP_MAX = 0xFFFFFFFF
};

/**
 * @brief
 *
 */
struct evt_context {
  /**
   * @brief
   *
   */
  evt_handle handle;

  /**
   * @brief
   *
   */
  void* userdata;

  /**
   * @brief Construct a new evt context object
   *
   * @param h
   * @param d
   */
  evt_context(evt_handle h, void* d)
      : handle(h)
      , userdata(d) {
  }
};

/**
 * @brief
 *
 */
struct evt_event {
  /**
   * @brief
   *
   */
  uint32_t flags;

  /**
   * @brief
   *
   */
  evt_context* context;

  /**
   * @brief Construct a new evt event object
   *
   * @param f
   * @param ctx
   */
  evt_event(uint32_t f, evt_context* ctx)
      : flags(f)
      , context(ctx) {
  }
};

/**
 * @brief
 *
 */
typedef std::vector<evt_event> evt_event_list;

namespace impl {
/**
 * @brief
 *
 */
class evt_impl {
public:
  /**
   * @brief Destroy the evt impl object
   *
   */
  virtual ~evt_impl(){};

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  virtual bool open() = 0;

  /**
   * @brief
   *
   * @param context
   * @param flags
   * @return true
   * @return false
   */
  virtual bool attach(evt_context* context, uint32_t flags) = 0;

  /**
   * @brief
   *
   * @param context
   * @return true
   * @return false
   */
  virtual bool detach(evt_context* context) = 0;

  /**
   * @brief
   *
   * @param event_list
   * @param max_count
   * @param timeout_ms
   * @return true
   * @return false
   */
  virtual bool wait(evt_event_list& event_list, int max_count, int timeout_ms) = 0;

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  virtual bool wakeup() = 0;

  /**
   * @brief
   *
   */
  virtual void close() = 0;
};
} // namespace impl

namespace {
/**
 * @brief
 *
 */
class logger {
public:
  /**
   * @brief
   *
   */
  class message {
  private:
    std::string tag_;
    std::ostringstream oss_;

  public:
    /**
     * @brief Constructs a new message object
     *
     * @param tag
     */
    message(const std::string& tag)
        : tag_(tag) {
      *this << "evtio[" << tag_ << "]";
    }

    /**
     * @brief
     *
     * @tparam T
     * @param msg
     * @return message&
     */
    template <class T>
    message& operator<<(const T& msg) {
      oss_ << msg;
      return *this;
    }

    /**
     * @brief
     *
     * @return std::ostringstream&
     */
    std::ostringstream& oss() {
      return oss_;
    }
  };

public:
  /**
   * @brief Constructs a new logger object
   *
   */
  logger(){};

  /**
   * @brief Destroys the logger object
   *
   */
  ~logger(){};

  /**
   * @brief
   *
   * @return logger&
   */
  static logger& instance() {
    static logger s_instance;
    return s_instance;
  }

  /**
   * @brief
   *
   * @param msg
   */
  void operator&=(message& msg) {
    std::cout << msg.oss().str() << std::endl;
  }
};

#if defined(__APPLE__)
/**
 * @brief
 *
 */
class evt_kqueue : public impl::evt_impl {
private:
  int kqfd_ = -1;
  int wakeup_evt_id_ = 1;

protected:
  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool kqueue_create() {
    if (kqfd_ >= 0) {
      return true;
    }

    kqfd_ = ::kqueue();
    if (kqfd_ < 0) {
      logE() << "failed to create kqueue instance:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    return true;
  }

  /**
   * @brief
   *
   */
  void kqueue_destroy() {
    if (kqfd_ < 0) {
      ::close(kqfd_);
    }
  }

  /**
   * @brief
   *
   * @param h
   * @param flags
   * @param context
   * @return true
   * @return false
   */
  bool kqueue_add(evt_handle h, int flags, evt_context* context) {
    // validate the arguments
    if (kqfd_ < 0 || (!(flags & op_read) && !(flags & op_write))) {
      logE() << "invalid kqueue instance";
      return false;
    }

    std::vector<struct kevent> evts;
    // build read event
    if (flags & op_read) {
      struct kevent ev;
      EV_SET(&ev, h, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, context);
      evts.push_back(ev);
    }

    // build write event
    if (flags & op_write) {
      struct kevent ev;
      EV_SET(&ev, h, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, context);
      evts.push_back(ev);
    }

    // associate the socket with kqueue read filter
    struct kevent ev;
    EV_SET(&ev, h, EVFILT_READ, EV_ADD, 0, 0, context);
    if (::kevent(kqfd_, evts.data(), evts.size(), nullptr, 0, nullptr) < 0) {
      logE() << "failed to associate the handle with kqueue read filter:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    return true;
  }

  /**
   * @brief
   *
   * @param h
   * @return true
   * @return false
   */
  bool kqueue_remove(evt_handle h) {
    if (kqfd_ < 0) {
      logE() << "invalid epoll instance:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    // remove the socket from the kqueue
    struct kevent ev;

    // remove from read filter
    EV_SET(&ev, h, EVFILT_READ, EV_DELETE, 0, 0, 0);
    if (0 != ::kevent(kqfd_, &ev, 1, nullptr, 0, nullptr) < 0 && errno == ENOENT) {
      logE() << "failed to remove the handle form the kqueue read list:"
             << "(" << errno << ")" << strerror(errno);
    }

    // remove from write filter
    EV_SET(&ev, h, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
    if (::kevent(kqfd_, &ev, 1, nullptr, 0, nullptr) < 0 && errno == ENOENT) {
      logE() << "failed to remove the handle form the kqueue write list:"
             << "(" << errno << ")" << strerror(errno);
    }

    return true;
  }

  /**
   * @brief
   *
   * @param event_list
   * @param max_count
   * @param timeout_ms
   * @return true
   * @return false
   */
  bool kqueue_wait(evt_event_list& event_list, int max_count, int timeout_ms) {
    // clear the result vector
    event_list.clear();

    struct timespec limit;
    limit.tv_sec = timeout_ms / 1000;
    limit.tv_nsec = (timeout_ms % 1000) * 1000000;

    // query the kqueue to get all signaled events
    std::vector<struct kevent> evts(max_count);
    int nfd = ::kevent(kqfd_, nullptr, 0, evts.data(), evts.size(), &limit);
    if (nfd < 0) {
      logE() << "failed to query the socket status from kqueue:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    for (int i = 0; i < nfd; i++) {
      // check the wakeup event
      if (evts[i].filter == EVFILT_USER && evts[i].ident == wakeup_evt_id_) {
        logI() << "kqueue wakeup event received";
        continue;
      }
      if (evts[i].filter == EVFILT_READ) {
        event_list.emplace_back(static_cast<uint32_t>(op_read),
                                static_cast<evt_context*>(evts[i].udata));
      } else if (evts[i].filter == EVFILT_WRITE) {
        event_list.emplace_back(static_cast<uint32_t>(op_write),
                                static_cast<evt_context*>(evts[i].udata));
      } else {
        logE() << "unknown event filter";
      }
    }
    return true;
  }

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool kqueue_add_wakeup_event() {
    struct kevent ev;
    EV_SET(&ev, wakeup_evt_id_, EVFILT_USER, EV_ADD | EV_CLEAR | EV_ENABLE, 0, 0, 0);
    if (::kevent(kqfd_, &ev, 1, nullptr, 0, nullptr) < 0) {
      logE() << "failed to associate the wakeup event with kqueue:"
             << "(" << errno << ")" << strerror(errno);
    }
  }

  /**
   * @brief
   *
   */
  void kqueue_remove_wakeup_event() {
    struct kevent ev;
    EV_SET(&ev, wakeup_evt_id_, EVFILT_USER, EV_DELETE | EV_DISABLE, 0, 0, 0);
    if (::kevent(kqfd_, &ev, 1, nullptr, 0, nullptr) < 0) {
      logE() << "failed to remove the wakeup event from the kqueue:"
             << "(" << errno << ")" << strerror(errno);
    }
  }

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool kqueue_wakeup() {
    struct kevent event;
    event.ident = wakeup_evt_id_;
    event.filter = EVFILT_USER;
    event.fflags = NOTE_TRIGGER;
    if (::kevent(kqfd_, &event, 1, nullptr, 0, nullptr) < 0) {
      logE() << "failed to signal the wakeup event for kqueue:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    return true;
  }

public:
  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool open() override {
    if (!kqueue_create()) {
      return false;
    };

    if (!kqueue_add_wakeup_event()) {
      kqueue_destroy();
      return false;
    }

    return true;
  }

  /**
   * @brief
   *
   * @param context
   * @param flags
   * @return true
   * @return false
   */
  bool attach(evt_context* context, uint32_t flags) override {
    if (!context || (!(flags & op_read) && !(flags & op_write))) {
      return false;
    }
    return kqueue_add(context->handle, flags, context);
  }

  /**
   * @brief
   *
   * @param context
   * @return true
   * @return false
   */
  bool detach(evt_context* context) override {
    if (!context) {
      return false;
    }
    return kqueue_remove(context->handle);
  }

  /**
   * @brief
   *
   * @param event_list
   * @param max_count
   * @param timeout_ms
   * @return true
   * @return false
   */
  bool wait(evt_event_list& event_list, int max_count, int timeout_ms) override {
    return kqueue_wait(event_list, max_count, timeout_ms);
  }

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool wakeup() override {
    return kqueue_wakeup();
  }

  /**
   * @brief
   *
   */
  void close() override {
    kqueue_remove_wakeup_event();
    kqueue_destroy();
  }
};
#endif

#if defined(__linux__)
/**
 * @brief
 *
 */
class evt_epoll : public impl::evt_impl {
private:
  int epfd_ = -1;
  int wakeup_fd_ = -1;

protected:
  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool epoll_create() {
    // exist already
    if (epfd_ >= 0) {
      return true;
    }

    // create epoll instance
    epfd_ = ::epoll_create(1024);
    if (epfd_ < 0) {
      logE() << "failed to create epoll instance:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    return true;
  }

  /**
   * @brief
   *
   */
  void epoll_destroy() {
    if (epfd_ < 0) {
      ::close(epfd_);
    }
  }

  /**
   * @brief
   *
   * @param h
   * @param flags
   * @param context
   * @return true
   * @return false
   */
  bool epoll_add(evt_handle h, int flags, evt_context* context) {
    // validate the epoll instance
    if (epfd_ < 0 || (!(flags & op_read) && !(flags & op_write))) {
      logE() << "invalid epoll instance";
      return false;
    }

    // associate the socket with epoll
    struct epoll_event ev;
    // !note we use level trigger here
    uint32_t f = 0;
    if (flags & op_read) {
      f |= EPOLLIN;
    }
    if (flags & op_write) {
      f |= EPOLLOUT;
    }

    ev.events = f;
    ev.data.ptr = context;
    if (0 != ::epoll_ctl(epfd_, EPOLL_CTL_ADD, h, &ev)) {
      logE() << "failed to associate the handle with epoll:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    return true;
  }

  /**
   * @brief
   *
   * @param h
   * @return true
   * @return false
   */
  bool epoll_remove(evt_handle h) {
    // validate the epoll instance
    if (epfd_ < 0) {
      logE() << "invalid epoll instance:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    // remove the socket from epoll
    if (0 != ::epoll_ctl(epfd_, EPOLL_CTL_DEL, h, nullptr)) {
      logE() << "failed to remove the handle from epoll:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }

    return true;
  }

  /**
   * @brief
   *
   * @param event_list
   * @param max_count
   * @param timeout_ms
   * @return true
   * @return false
   */
  bool epoll_wait(evt_event_list& event_list, int max_count, int timeout_ms) {
    // clear the result vector
    event_list.clear();

    // query the epoll to get all signaled events
    std::vector<struct epoll_event> evts(max_count);
    int nfd = ::epoll_wait(epfd_, evts.data(), evts.size(), timeout_ms);
    if (nfd < 0) {
      logE() << "failed to query the handle from the epoll:"
             << "(" << errno << ")" << strerror(errno);
    }

    // process the result
    for (int i = 0; i < nfd; i++) {
      // validate the event data
      if (!evts[i].data.ptr) {
        continue;
      }

      // read eventfd to reset its status
      if (evts[i].data.ptr == nullptr) {
        uint64_t n;
        ::read(wakeup_fd_, &n, sizeof(uint64_t));
      }

      // build event
      uint32_t flags = 0;
      if (evts[i].events & EPOLLIN) {
        flags |= op_read;
      }
      if (evts[i].events & EPOLLOUT) {
        flags |= op_write;
      }
      if (flags == 0) {
        // logE() << "unknown event flags: " << evts[i].events;
        continue;
      }

      // push back the event
      event_list.emplace_back(flags, static_cast<evt_context*>(evts[i].data.ptr));
    }

    return true;
  }

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool epoll_add_wakeup_event() {
    wakeup_fd_ = eventfd(0, EFD_NONBLOCK);
    if (wakeup_fd_ < 0) {
      logE() << "failed to create eventfd:"
             << "(" << errno << ")" << strerror(errno);
      return false;
    }
    return epoll_add(wakeup_fd_, op_read, nullptr);
  }

  /**
   * @brief
   *
   */
  void epoll_remove_wakeup_event() {
    epoll_remove(wakeup_fd_);
  }

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool epoll_wakeup() {
    uint64_t n = 1;
    return (sizeof(uint64_t) == write(wakeup_fd_, &n, sizeof(uint64_t)));
  }

public:
  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool open() override {
    if (!epoll_create()) {
      return false;
    };

    if (!epoll_add_wakeup_event()) {
      epoll_destroy();
      return false;
    }

    return true;
  }

  /**
   * @brief
   *
   * @param context
   * @param flags
   * @return true
   * @return false
   */
  bool attach(evt_context* context, uint32_t flags) override {
    if (!context || (!(flags & op_read) && !(flags & op_write))) {
      return false;
    }
    return epoll_add(context->handle, flags, context);
  }

  /**
   * @brief
   *
   * @param context
   * @return true
   * @return false
   */
  bool detach(evt_context* context) override {
    if (!context) {
      return false;
    }
    return epoll_remove(context->handle);
  }

  /**
   * @brief
   *
   * @param event_list
   * @param max_count
   * @param timeout_ms
   * @return true
   * @return false
   */
  bool wait(evt_event_list& event_list, int max_count, int timeout_ms) override {
    return epoll_wait(event_list, max_count, timeout_ms);
  }

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool wakeup() override {
    return epoll_wakeup();
  }

  /**
   * @brief
   *
   */
  void close() override {
    epoll_remove_wakeup_event();
    epoll_destroy();
  }
};
#endif

#if defined(_WIN32)
class evt_iocp : public impl::evt_impl {
public:
  bool open() override {
    throw std::logic_error("method not yet implemented");
  }

  bool attach(evt_context* context) override {
    throw std::logic_error("method not yet implemented");
  }

  bool detach(evt_context* context) override {
    throw std::logic_error("method not yet implemented");
  }

  bool wait(evt_context_list& context_list, int max_count, int timeout_ms) override {
    throw std::logic_error("method not yet implemented");
  }

  bool wakeup() override {
    throw std::logic_error("method not yet implemented");
  }

  void close() override {
    throw std::logic_error("method not yet implemented");
  }
};
#endif
} // namespace

/**
 * @brief
 *
 */
class evt {
public:
  /**
   * @brief Constructs a new evt object
   *
   */
  evt()
      :
#if defined(__linux__)
      impl_(new evt_epoll())
#elif defined(__APPLE__)
      impl_(new evt_kqueue())
#elif defined(_WIN32)
      impl_(new evt_iocp())
#else
#error "unsupported platform"
#endif
  {
  }

  /**
   * @brief Destroys the evt object
   *
   */
  ~evt() {
  }

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool open() {
    return impl_->open();
  }

  /**
   * @brief
   *
   * @param context
   * @param flags
   * @return true
   * @return false
   */
  bool attach(evt_context* context, uint32_t flags) {
    return impl_->attach(context, flags);
  }

  /**
   * @brief
   *
   * @param context
   * @return true
   * @return false
   */
  bool detach(evt_context* context) {
    return impl_->detach(context);
  }

  /**
   * @brief
   *
   * @param event_list
   * @param max_count
   * @param timeout_ms
   * @return true
   * @return false
   */
  bool wait(evt_event_list& event_list, int max_count, int timeout_ms) {
    return impl_->wait(event_list, max_count, timeout_ms);
  }

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool wakeup() {
    return impl_->wakeup();
  }

  /**
   * @brief
   *
   */
  void close() {
    return impl_->close();
  }

private:
  /**
   * @brief
   *
   */
  std::unique_ptr<impl::evt_impl> impl_;
};
#endif
} // namespace evtio
