#pragma once

class IRunnable
{
public:
    virtual ~IRunnable() = default;

    virtual void run() = 0;
};