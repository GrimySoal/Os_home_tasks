#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <utility>
#include <mutex>
#include <stdexcept>
#include <thread>

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
    send_locker.lock();
    locker.lock();
    buffer[real_amount] = value;
    ++real_amount;
    locker.unlock();
    if(real_amount < size) send_locker.unlock();
    recv_locker.unlock();
  }

  std::pair<T, bool> recv() {
    if (is_closed && real_amount == 0) {
      return {T(), false};
    }
    recv_locker.lock();
    locker.lock();
    if(real_amount == 0){
      locker.unlock();
      return {T(), false};
    }
    T temp = std::move(buffer[real_amount - 1]);
    --real_amount;
    locker.unlock();
    send_locker.unlock();
    if(real_amount > 0) recv_locker.unlock();
    return {temp, true};
  }

  void close() {
    is_closed = true;
    unsigned int threads_amount = std::thread::hardware_concurrency();
    threads_amount = threads_amount == 0 ? 50 : threads_amount;
    //I know it is really workaround but I was interested in doing this without conditional variables
    for(int i = 0; i < threads_amount; ++i){
      recv_locker.unlock();
    }
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
