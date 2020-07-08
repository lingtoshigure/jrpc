#pragma once

#include <memory>
#include <set>

#include <libnet/Timer.h>
#include <libnet/Channel.h>
#include <libnet/Timestamp.h>

namespace net
{

class TimerQueue: noncopyable
{
public:
  explicit
  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  Timer*  addTimer(TimerCallback cb, Timestamp when, Microsecond interval);
  void    cancelTimer(Timer* timer);

  int64_t nextTimeout() const 
  { 
    if(timers_.empty()) 
      return 0;
      
    auto interval = timers_.begin()->first - clock::now();
    return std::chrono::duration_cast<Millisecond>(interval).count();
  }

private:
  using Entry     = std::pair<Timestamp, Timer*>;
  using TimerList = std::set<Entry>;

  void handleRead();
  std::vector<Entry> getExpired(Timestamp now);

  EventLoop* loop_;
  const int  timerfd_;
  Channel    timerChannel_;
  TimerList  timers_;
};

}
