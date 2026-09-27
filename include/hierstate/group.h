#pragma once

#include <hierstate/state.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hierstate
{

template<class Id>
class group : public state
{
    static_assert(std::is_enum_v<Id>, "group Id must be an enum");

public:
    void update() override;
    void enter() noexcept override;
    void exit() noexcept override;

    bool request(Id id);

    std::optional<Id> current() const;
    const state* get() const noexcept;

protected:
    void start(Id id);
    virtual std::unique_ptr<state> create(Id id) = 0;

private:
    std::unique_ptr<state> child;
    std::optional<Id> curr;
    std::optional<Id> next;
    bool entered = false;
};

template <class Id>
inline void group<Id>::start(Id id)
{
    if (child)
    {
        throw std::logic_error("group already started");
    }

    auto candidate = create(id);
    if (!candidate)
    {
        throw std::logic_error("create returned null");
    }

    child = std::move(candidate);
    curr = id;
}

template <class Id>
inline bool group<Id>::request(Id id)
{
    if (next.has_value())
    {
        return false;
    }

    next = id;
    return true;
}

template <class Id>
inline std::optional<Id> group<Id>::current() const
{
    return curr;
}

template <class Id>
inline const state *group<Id>::get() const noexcept
{
    return child.get();
}

template <class Id>
inline void group<Id>::update()
{
    if (!child)
    {
        throw std::logic_error("group has no child");
    }
    if (!entered)
    {
        enter();
    }

    child->update();

    if (next.has_value())
    {
        auto candidate = create(*next);
        if (!candidate)
        {
            throw std::logic_error("create returned null");
        }

        child->exit();
        child = std::move(candidate);

        curr = *next;
        next.reset();

        child->enter();
    }
}

template <class Id>
inline void group<Id>::enter() noexcept
{
    if (!child)
    {
        return;
    }
    if (entered)
    {
        return;
    }

    child->enter();
    entered = true;
}

template <class Id>
inline void group<Id>::exit() noexcept
{
    if (!child)
    {
        return;
    }
    if (!entered)
    {
        return;
    }

    child->exit();
    entered = false;
}

} // namespace hierstate
