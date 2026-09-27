#include <hierstate/group.h>

#include <iostream>
#include <memory>
#include <vector>

namespace
{

enum class outer
{
    a,
    branch,
};

enum class inner
{
    b,
    c,
};

enum class event
{
    a_enter,
    a_update,
    a_end,
    a_exit,
    a_destroy,
    b_enter,
    b_update,
    b_end,
    b_exit,
    b_destroy,
    c_enter,
    c_update,
    c_end,
    c_exit,
    c_destroy,
    branch_enter,
    branch_exit,
    branch_destroy,
};

using history = std::vector<event>;

class a : public hierstate::state
{
public:
    a(hierstate::group<outer>& parent, history& events)
        : parent(parent), events(events)
    {
    }

    ~a() override
    {
        events.push_back(event::a_destroy);
    }

    void enter() noexcept override
    {
        events.push_back(event::a_enter);
    }

    void update() override
    {
        events.push_back(event::a_update);
        parent.request(outer::branch);
        events.push_back(event::a_end);
    }

    void exit() noexcept override
    {
        events.push_back(event::a_exit);
    }

private:
    hierstate::group<outer>& parent;
    history& events;
};

class b : public hierstate::state
{
public:
    b(hierstate::group<inner>& parent, history& events)
        : parent(parent), events(events)
    {
    }

    ~b() override
    {
        events.push_back(event::b_destroy);
    }

    void enter() noexcept override
    {
        events.push_back(event::b_enter);
    }

    void update() override
    {
        events.push_back(event::b_update);
        parent.request(inner::c);
        events.push_back(event::b_end);
    }

    void exit() noexcept override
    {
        events.push_back(event::b_exit);
    }

private:
    hierstate::group<inner>& parent;
    history& events;
};

class c : public hierstate::state
{
public:
    c(hierstate::group<outer>& root, history& events)
        : root(root), events(events)
    {
    }

    ~c() override
    {
        events.push_back(event::c_destroy);
    }

    void enter() noexcept override
    {
        events.push_back(event::c_enter);
    }

    void update() override
    {
        events.push_back(event::c_update);
        root.request(outer::a);
        events.push_back(event::c_end);
    }

    void exit() noexcept override
    {
        events.push_back(event::c_exit);
    }

private:
    hierstate::group<outer>& root;
    history& events;
};

class branch : public hierstate::group<inner>
{
public:
    branch(hierstate::group<outer>& root, history& events)
        : root(root), events(events)
    {
        start(inner::b);
    }

    ~branch() override
    {
        events.push_back(event::branch_destroy);
    }

    void enter() noexcept override
    {
        events.push_back(event::branch_enter);
        hierstate::group<inner>::enter();
    }

    void exit() noexcept override
    {
        hierstate::group<inner>::exit();
        events.push_back(event::branch_exit);
    }

protected:
    std::unique_ptr<hierstate::state> create(inner id) override
    {
        switch (id)
        {
        case inner::b:
            return std::make_unique<b>(*this, events);
        case inner::c:
            return std::make_unique<c>(root, events);
        }
        return nullptr;
    }

private:
    hierstate::group<outer>& root;
    history& events;
};

class root : public hierstate::group<outer>
{
public:
    explicit root(history& events)
        : events(events)
    {
        start(outer::a);
    }

protected:
    std::unique_ptr<hierstate::state> create(outer id) override
    {
        switch (id)
        {
        case outer::a:
            return std::make_unique<a>(*this, events);
        case outer::branch:
            return std::make_unique<branch>(*this, events);
        }
        return nullptr;
    }

private:
    history& events;
};

} // namespace

int main()
{
    history events;
    events.reserve(64);
    {
        root machine(events);
        if (machine.current() != outer::a)
        {
            std::cerr << "root did not start in a\n";
            return 1;
        }
        machine.update();
        auto* active_branch = dynamic_cast<const branch*>(machine.get());
        if (machine.current() != outer::branch || !active_branch || active_branch->current() != inner::b)
        {
            std::cerr << "a did not transition to branch/b\n";
            return 1;
        }
        machine.update();
        active_branch = dynamic_cast<const branch*>(machine.get());
        if (machine.current() != outer::branch || !active_branch || active_branch->current() != inner::c)
        {
            std::cerr << "branch did not transition from b to c\n";
            return 1;
        }
        machine.update();
        if (machine.current() != outer::a)
        {
            std::cerr << "c did not transition to a\n";
            return 1;
        }
        machine.update();
        machine.exit();
        machine.exit();
    }

    const history expected{
        event::a_enter, event::a_update, event::a_end,
        event::a_exit, event::a_destroy,
        event::branch_enter, event::b_enter,
        event::b_update, event::b_end, event::b_exit,
        event::b_destroy, event::c_enter,
        event::c_update, event::c_end, event::c_exit,
        event::branch_exit, event::branch_destroy, event::c_destroy,
        event::a_enter,
        event::a_update, event::a_end, event::a_exit,
        event::a_destroy, event::branch_enter, event::b_enter,
        event::b_exit, event::branch_exit, event::branch_destroy,
        event::b_destroy,
    };

    if (events != expected)
    {
        std::cerr << "nested transition or destruction order differs\n";
        return 1;
    }
}
