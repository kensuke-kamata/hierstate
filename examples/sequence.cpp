#include <hierstate.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>

enum class scene
{
    title,
    game,
};

enum class phase
{
    play,
    clear,
};

class title : public hierstate::state
{
public:
    explicit title(hierstate::group<scene>& parent)
        : parent(parent)
    {
    }

    ~title()
    {
        std::cout << "title destroyed\n";
    }

    void enter() noexcept override
    {
        std::puts("title enter");
    }

    void update() override
    {
        std::cout << "title update\n";
        parent.request(scene::game);

        std::cout << "title update end\n";
    }

    void exit() noexcept override
    {
        std::puts("title exit");
    }

private:
    hierstate::group<scene>& parent;
};

class play : public hierstate::state
{
public:
    explicit play(hierstate::group<phase>& parent)
        : parent(parent)
    {
    }

    ~play()
    {
        std::cout << "play destroyed\n";
    }

    void enter() noexcept override
    {
        std::puts("play enter");
    }

    void update() override
    {
        std::cout << "play update\n";
        parent.request(phase::clear);

        std::cout << "play update end\n";
    }

    void exit() noexcept override
    {
        std::puts("play exit");
    }

private:
    hierstate::group<phase>& parent;
};

class clear : public hierstate::state
{
public:
    explicit clear(hierstate::group<scene>& parent)
        : parent(parent)
    {
    }

    ~clear()
    {
        std::cout << "clear destroyed\n";
    }

    void enter() noexcept override
    {
        std::puts("clear enter");
    }

    void update() override
    {
        std::cout << "clear update\n";
        parent.request(scene::title);

        std::cout << "clear update end\n";
    }

    void exit() noexcept override
    {
        std::puts("clear exit");
    }

private:
    hierstate::group<scene>& parent;
};

class game : public hierstate::group<phase>
{
public:
    explicit game(hierstate::group<scene>& parent)
        : parent(parent)
    {
        start(phase::play);
    }

    void enter() noexcept override
    {
        std::puts("game enter");
        hierstate::group<phase>::enter();
    }

    void exit() noexcept override
    {
        hierstate::group<phase>::exit();
        std::puts("game exit");
    }

protected:
    virtual std::unique_ptr<state> create(phase id) override
    {
        switch (id)
        {
        case phase::play:
            return std::make_unique<play>(*this);
        case phase::clear:
            return std::make_unique<clear>(parent);
        }
        throw std::invalid_argument("unknown phase");
    }

private:
    hierstate::group<scene>& parent;
};

class root : public hierstate::group<scene>
{
public:
    root()
    {
        start(scene::title);
    }

protected:
    virtual std::unique_ptr<state> create(scene id) override
    {
        switch (id)
        {
        case scene::title:
            return std::make_unique<title>(*this);
        case scene::game:
            return std::make_unique<game>(*this);
        }
        throw std::invalid_argument("unknown scene");
    }
};

void show(const root& r)
{
    if (r.current() == scene::title)
    {
        std::cout << "current: title\n";
        return;
    }

    const auto* g = dynamic_cast<const game*>(r.get());
    if (!g)
    {
        throw std::logic_error("expected game child");
    }

    if (g->current() == phase::play)
    {
        std::cout << "current: game / play\n";
    }
    else if (g->current() == phase::clear)
    {
        std::cout << "current: game / clear\n";
    }
    else
    {
        throw std::logic_error("expected play or clear phase");
    }
}

int main()
{
    std::cout << "hierstate sequence example\n";

    root r;
    show(r);

    r.update();
    show(r);

    r.update();
    show(r);

    r.update();
    show(r);

    r.update();
    show(r);

    r.exit();

    return 0;
}
