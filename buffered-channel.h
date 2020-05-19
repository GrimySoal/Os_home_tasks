#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <utility>
#include <mutex>
#include <stdexcept>

template<class T>
class BufferedChannel {
 public:
  explicit BufferedChannel(const int size_) {
    size = size_;
    buffer = new T[size_];
    recv_locker.lock();
  }

  void send(const T value) {
    if (is_closed) throw std::runtime_error("channel is closed");
    locker.lock();
    send_locker.lock();
    if (is_closed) {
      locker.unlock();
      send_locker.unlock();
      throw std::runtime_error("channel is closed");
    }
    buffer[real_amount] = value;
    if(++real_amount < size) send_locker.unlock();
    recv_locker.unlock();
    locker.unlock();
  }

  std::pair<T, bool> recv() {
    if (is_closed && real_amount == 0) {
      return {T(), false};
    }
    locker.lock();
    recv_locker.lock();
    if(real_amount == 0){
      locker.unlock();
      recv_locker.unlock();
      return {T(), false};
    }
    T temp = std::move(buffer[real_amount - 1]);
    if(--real_amount > 0 || is_closed) recv_locker.unlock();
    send_locker.unlock();
    locker.unlock();
    return {temp, true};
  }

  void close() {
    is_closed = true;
    recv_locker.unlock();
    send_locker.unlock();
  }

 private:
  std::mutex locker;
  std::mutex send_locker;
  std::mutex recv_locker;
  int real_amount = 0;
  int size;
  bool is_closed = false;
  T* buffer;
};

#endif // BUFFERED_CHANNEL_H_
