#include <chrono>
#include <coroutine>
#include <exception>
#include <future>
#include <iostream>
#include <thread>
#include <type_traits>
 
// Enable the use of std::future<T> as a coroutine type
// by using a std::promise<T> as the promise type.
template <typename T, typename... Args>
struct std::coroutine_traits<std::future<T>, Args...> {
  struct promise_type : std::promise<T> {
      // Write here your code
  };
};
 

 
// Allow co_await'ing std::future<T>
// by naively spawning a new thread for each co_await.
template <typename T>
auto operator co_await(std::future<T> future) noexcept
requires(!std::is_reference_v<T>) {
  struct awaiter : std::future<T> {
    bool await_ready() const noexcept {
      using namespace std::chrono_literals;
      return this->wait_for(0s) != std::future_status::timeout;
    }
    void await_suspend(std::coroutine_handle<> cont) const {
      std::thread([this, cont] {
        this->wait();
        cont();
      }).detach();
    }
    T await_resume() { return this->get(); }
  };
  return awaiter{std::move(future)};
}
 
// Utilize the infrastructure we have established.
std::future<int> compute() {
  int a = co_await std::async([] { return 6; });
  int b = co_await std::async([] { return 7; });
  co_return a * b;
}
 
int main() {
  std::cout << compute().get() << '\n';
}
