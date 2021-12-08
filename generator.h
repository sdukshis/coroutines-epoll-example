#pragma once

#include <coroutine>

template<typename T>
struct generator {
    // We are a "resumable thing" and this is our promise
    struct promise_type {
        T const *_current;

        // Required to provide for expression "co_yield value" (See https://youtu.be/ZTqHjjm86Bw?t=2463)
        // I.e., Compiler replaces "co_yield value" with "co_await __promise.yeld_value(value)" and here we define it.
        auto yield_value(const T &value) {
            _current = &value;
            return std::suspend_always{};
        }

        auto initial_suspend() {
            // Always initially suspend to allow each value (inclduing the first ione) to be yielded as result of resume
            return std::suspend_always{};
        }

        generator get_return_object() {
            return generator(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        auto final_suspend() noexcept {
            // Always suspend before exiting coroutine to allow last yelded value to be retrieved by client
            return std::suspend_always{};
        }

        void unhandled_exception() { std::terminate(); }

        void return_void() {}
    };

    // We are a constructable and destroyable "resumable_thing" (See https://youtu.be/ZTqHjjm86Bw?t=1119)
    std::coroutine_handle<promise_type> _coroutine;

    explicit generator(std::coroutine_handle<promise_type> coroutine)
            : _coroutine(coroutine) {}

    ~generator() {
        if (_coroutine) { _coroutine.destroy(); }
    }

    // Only move allowed (resumable_thing cloning not allowed)
    generator(generator &&other) : _coroutine(std::move(other._coroutine)) {
        // IMPORTANT: nullify the moved-from coroutine handle (See https://youtu.be/ZTqHjjm86Bw?t=1153)
        // The std::coroutine_handle<> type does NOT provide any life-time support
        other._coroutine = nullptr; // moved-from now null
    }

    generator operator=(const generator &other) = delete;

    generator operator=(generator &&other) = delete;

    // Make us a callable resumable_thing (operate as generator)
    T operator()() {
        if (!_coroutine.done()) {
            _coroutine.resume();
            return *_coroutine.promise()._current;
        } else {
            // Failsafe if resumable generator exited before the client stopped calling us
            return T{-1, -1};
        }
    }

    // Make us an iteratable resumable_thing (Se we can be resumed in e.g., a for loop)
    struct iterator : public std::iterator<std::input_iterator_tag, T> {
        std::coroutine_handle<promise_type> _coroutine;

        iterator() = default;

        iterator(std::coroutine_handle<promise_type> coroutine)
                : _coroutine(coroutine) {}

        iterator &operator++() {
            _coroutine.resume();
            if (_coroutine.done()) { _coroutine = nullptr; }
            return *this;
        }

        T const &operator*() {
            return *_coroutine.promise()._current;
        }

        bool operator!=(const iterator &other) const {
            return (_coroutine != other._coroutine);
        }
    };

    iterator begin() {
        if (_coroutine) {
            _coroutine.resume();
            if (_coroutine.done()) { return end(); }
        }
        return iterator(_coroutine);
    }

    iterator end() { return iterator{}; }
};
