#pragma once

class ITask
{
public:
    virtual ~ITask() = default;

    virtual void run() = 0;
};