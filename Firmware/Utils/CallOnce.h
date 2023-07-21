#pragma once

#include <functional>
#include <utility>

template <typename Func>
class CallOnce
{
public:
    CallOnce() = default;

    explicit CallOnce(Func&& func)
        : _func{ std::move(func) }
    {}

    void callIfSignaled()
    {
        if (_funcWrapper) {
            decltype(_funcWrapper) funcWrapper;
            std::swap(funcWrapper, _funcWrapper);
            funcWrapper();
        }
    }

    template <typename... Args>
    void signal(Args... args)
    {
        if (_func) {
            _funcWrapper = [...args{ std::forward<Args>(args) }, this]() mutable {
                _func(std::forward<Args>(args)...);
            };
        }
    }

private:
    Func _func;
    std::function<void ()> _funcWrapper;
    bool _call{ false };
};