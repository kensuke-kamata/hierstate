#pragma once

namespace hierstate
{

class state
{
public:
    virtual ~state() = default;
    virtual void update() = 0;
    virtual void enter() noexcept {}
    virtual void exit() noexcept {}
};

}
