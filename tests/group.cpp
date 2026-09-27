#include <hierstate/group.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

static_assert(noexcept(std::declval<hierstate::state&>().enter()));
static_assert(noexcept(std::declval<hierstate::state&>().exit()));

namespace
{

enum class choice
{
    active,
    absent,
};

static_assert(noexcept(std::declval<hierstate::group<choice>&>().enter()));
static_assert(noexcept(std::declval<hierstate::group<choice>&>().exit()));

struct counts
{
    int enters = 0;
    int updates = 0;
    int exits = 0;
    int destroys = 0;
    bool updated_without_enter = false;
};

class counter : public hierstate::state
{
public:
    explicit counter(counts& observed)
        : observed(observed)
    {
    }

    ~counter() override
    {
        ++observed.destroys;
    }

    void update() override
    {
        if (observed.enters == 0)
        {
            observed.updated_without_enter = true;
        }
        ++observed.updates;
    }

    void enter() noexcept override
    {
        ++observed.enters;
    }

    void exit() noexcept override
    {
        ++observed.exits;
    }

private:
    counts& observed;
};

class fixture : public hierstate::group<choice>
{
public:
    explicit fixture(counts& observed)
        : observed(observed)
    {
    }

    void begin(choice id)
    {
        start(id);
    }

protected:
    std::unique_ptr<hierstate::state> create(choice id) override
    {
        if (id == choice::active)
        {
            return std::make_unique<counter>(observed);
        }
        return nullptr;
    }

private:
    counts& observed;
};

template<class Action>
bool throws(Action&& action)
{
    try
    {
        action();
    }
    catch (const std::logic_error&)
    {
        return true;
    }
    return false;
}

} // namespace

int main()
{
    counts observed;

    {
        fixture group(observed);
        if (group.current().has_value() || group.get() != nullptr)
        {
            std::cerr << "group reported a current child before start\n";
            return 1;
        }
        group.enter();
        group.exit();
        if (!throws([&] { group.update(); }))
        {
            std::cerr << "update before start did not fail\n";
            return 1;
        }
        if (!throws([&] { group.begin(choice::absent); }))
        {
            std::cerr << "null initial child did not fail\n";
            return 1;
        }
        if (group.current().has_value() || group.get() != nullptr || observed.updates != 0 || observed.destroys != 0)
        {
            std::cerr << "failed start changed child state\n";
            return 1;
        }
    }

    {
        fixture group(observed);
        group.begin(choice::active);
        const hierstate::state* initial_child = group.get();
        if (group.current() != choice::active || initial_child == nullptr)
        {
            std::cerr << "start did not record the current child\n";
            return 1;
        }

        if (!group.request(choice::absent))
        {
            std::cerr << "first transition request was rejected\n";
            return 1;
        }
        if (!throws([&] { group.update(); }))
        {
            std::cerr << "null replacement did not fail\n";
            return 1;
        }
        if (group.current() != choice::active || group.get() != initial_child || observed.updates != 1 || observed.exits != 0 || observed.destroys != 0)
        {
            std::cerr << "failed replacement exited or lost the active child\n";
            return 1;
        }
        if (group.request(choice::active))
        {
            std::cerr << "failed replacement cleared the pending request\n";
            return 1;
        }
        if (!throws([&] { group.update(); }) || observed.updates != 2)
        {
            std::cerr << "active child was not retained after failure\n";
            return 1;
        }
    }

    if (observed.destroys != 1)
    {
        std::cerr << "active child was not destroyed with its group\n";
        return 1;
    }

    counts repeated;

    {
        fixture group(repeated);
        group.begin(choice::active);

        if (!throws([&] { group.begin(choice::active); }))
        {
            std::cerr << "second start did not fail\n";
            return 1;
        }

        group.update();
        if (repeated.updates != 1 || repeated.destroys != 0)
        {
            std::cerr << "second start changed the active child\n";
            return 1;
        }
    }

    if (repeated.destroys != 1)
    {
        std::cerr << "active child was not destroyed with its group\n";
        return 1;
    }

    counts initial;

    {
        fixture group(initial);
        group.begin(choice::active);
        if (initial.enters != 0)
        {
            std::cerr << "start entered the child before update\n";
            return 1;
        }

        group.update();
        group.update();
        group.enter();

        if (initial.enters != 1 || initial.updates != 2 || initial.updated_without_enter)
        {
            std::cerr << "initial child was not entered exactly once before update\n";
            return 1;
        }
    }
}
