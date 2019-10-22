/*
 * This file is part of loxx.
 *
 * loxx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * loxx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created by Matt Spraggs on 21/10/2019.
 */

#ifndef LOXX_ASYNCWORKER_HPP
#define LOXX_ASYNCWORKER_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>


namespace loxx
{
  template <typename Fn, std::size_t QSize>
  class AsyncWorker
  {
  public:
    AsyncWorker();
    virtual ~AsyncWorker();

    std::size_t queue_size() const { return queue_size_; }

  protected:
    void push(Fn task);

    template <typename... Args>
    void emplace(Args&&... args);

  private:
    void main();
    Fn pop();

    std::atomic_bool quit_;
    std::size_t head_pos_, tail_pos_;
    std::atomic_size_t queue_size_;
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::array<Fn, QSize> queue_;
  };


  template <typename Fn, std::size_t QSize>
  AsyncWorker<Fn, QSize>::AsyncWorker()
      : quit_(false), head_pos_(0), tail_pos_(0), queue_size_(0),
        worker_([&] () { main(); })
  {
  }


  template <typename Fn, std::size_t QSize>
  AsyncWorker<Fn, QSize>::~AsyncWorker()
  {
    std::unique_lock<std::mutex> lock(mutex_);

    quit_ = true;

    lock.unlock();
    cv_.notify_all();

    if (worker_.joinable()) {
      worker_.join();
    }
  }


  template <typename Fn, std::size_t QSize>
  void AsyncWorker<Fn, QSize>::push(Fn task)
  {
    if (queue_size_ == QSize) {
      throw std::overflow_error("Task queue overflow.");
    }

    std::unique_lock<std::mutex> lock(mutex_);

    queue_[tail_pos_] = std::move(task);
    tail_pos_ = (tail_pos_ + 1) % QSize;
    ++queue_size_;

    lock.unlock();
    cv_.notify_one();
  }


  template <typename Fn, std::size_t QSize>
  template <typename... Args>
  void AsyncWorker<Fn, QSize>::emplace(Args&&... args)
  {
    if (queue_size_ == QSize) {
      std::overflow_error("Task queue overflow.");
    }

    std::unique_lock<std::mutex> lock(mutex_);

    queue_[tail_pos_] = Fn(std::forward<Args>(args)...);
    tail_pos_ = (tail_pos_ + 1) % QSize;
    ++queue_size_;

    lock.unlock();
    cv_.notify_one();
  }


  template <typename Fn, std::size_t QSize>
  void AsyncWorker<Fn, QSize>::main()
  {
    std::unique_lock<std::mutex> lock(mutex_);

    while (not quit_) {
      cv_.wait(lock, [this] { return quit_ or queue_size_ > 0; });

      if (not quit_ and queue_size_ > 0) {
        const auto task = pop();

        lock.unlock();

        task();

        lock.lock();
      }
    }
  }


  template <typename Fn, std::size_t QSize>
  Fn AsyncWorker<Fn, QSize>::pop()
  {
    const auto ret = std::move(queue_[head_pos_]);
    head_pos_ = (head_pos_ + 1) % QSize;
    --queue_size_;

    return ret;
  }
}

#endif // LOXX_ASYNC_WORKER_HPP
 