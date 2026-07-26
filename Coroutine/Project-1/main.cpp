#include <iostream>
#include <coroutine>

using namespace std;

namespace normalFunction
{
    void printNumbers()
    {
        cout << 1 << '\n';
        cout << 2 << '\n';
        cout << 3 << '\n';
    }
}

namespace coroutineExample1
{
    // This example only shows co_return.
    // It doesn't pause, so it's not very interesting.
    struct Task
    {
        struct promise_type
        {
            Task get_return_object()
            {
                cout << "get_return_object\n";
                return {};
            }

            std::suspend_never initial_suspend()
            {
                cout << "initial_suspend\n";
                return {};
            }

            std::suspend_never final_suspend() noexcept
            {
                cout << "final_suspend\n";
                return {};
            }

            void return_void()
            {
                cout << "return_void\n";
            }

            void unhandled_exception()
            {
                cout << "unhandled_exception\n";
            }
        };
    };

    Task hello()
    {
        std::cout << "Hello\n";
        co_return;
    }
}

namespace coroutineExample2
{
    /*
        Small Example with Pause (co_yield)
        This is the classic use of coroutines:
        generate one value at a time while automatically remembering where execution stopped.
    */
    class Generator
    {
    public:
        struct promise_type;
        using Handle = std::coroutine_handle<promise_type>;

        struct promise_type
        {
            int current;

            Generator get_return_object()
            {
                cout << "get_return_object\n";
                return Generator{Handle::from_promise(*this)};
            }

            std::suspend_always initial_suspend()
            {
                cout << "initial_suspend\n";
                return {};
            }

            std::suspend_always final_suspend() noexcept
            {
                cout << "final_suspend\n";
                return {};
            }

            std::suspend_always yield_value(int value)
            {
                cout << "yield_value\n";
                current = value;
                return {};
            }

            void return_void()
            {
                cout << "return_void\n";
            }

            void unhandled_exception()
            {
                cout << "unhandled_exception\n";
                std::terminate();
            }
        };

        explicit Generator(Handle h) : handle(h)
        {
            cout << "Generator\n";
        }

        ~Generator()
        {
            cout << "~Generator\n";

            if (handle)
                handle.destroy();
        }

        bool next()
        {
            cout << "next\n";
            handle.resume();
            return !handle.done();
        }

        int value() const
        {
            cout << "value\n";
            return handle.promise().current;
        }

    private:
        Handle handle;
    };

    Generator numbers()
    {
        cout << "numbers\n";
        co_yield 1;
        co_yield 2;
        co_yield 3;
    }

    void test()
    {
        cout << "test\n";

        auto g = numbers();

        while (g.next())
            std::cout << g.value() << '\n';
    }
}

namespace coroutineExample3
{
    /*
        co_await is the C++20 keyword that pauses a coroutine without blocking the thread.
        When the awaited operation finishes, the coroutine resumes from the point where it was suspended.
    */
    struct Task
    {
        struct promise_type
        {
            Task get_return_object()
            {
                return {};
            }

            std::suspend_never initial_suspend()
            {
                return {};
            }

            std::suspend_never final_suspend() noexcept
            {
                return {};
            }

            void return_void()
            {
            }

            void unhandled_exception()
            {
            }
        };
    };

    // Object used with co_await
    struct MyAwaiter
    {
        bool await_ready()
        {
            return false; // Suspend the coroutine
        }

        void await_suspend(std::coroutine_handle<>)
        {
            std::cout << "Coroutine suspended\n";
        }

        void await_resume()
        {
            std::cout << "Coroutine resumed\n";
        }
    };

    Task example()
    {
        std::cout << "Before co_await\n";

        co_await MyAwaiter{};

        std::cout << "After co_await\n";
    }

}

namespace coroutineExample4
{
    // Resuming co_await

    std::coroutine_handle<> saved;

    struct Task
    {
        struct promise_type
        {
            Task get_return_object()
            {
                return {};
            }

            std::suspend_never initial_suspend()
            {
                return {};
            }

            std::suspend_never final_suspend() noexcept
            {
                return {};
            }

            void return_void()
            {
            }

            void unhandled_exception()
            {
            }
        };
    };

    struct MyAwaiter
    {
        bool await_ready()
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> h)
        {
            std::cout << "Coroutine suspended\n";
            /*
                Resume immediately, restarts the coroutine exactly where it stopped.
                Normally you don't call resume() immediately. 
                Instead, you save the handle and resume it later.
            */
            // h.resume();

            saved = h; // Save it for later
        }

        void await_resume()
        {
            std::cout << "Coroutine resumed\n";
        }
    };

    Task example()
    {
        std::cout << "Before co_await\n";

        co_await MyAwaiter{};

        std::cout << "After co_await\n";
    }

    void test()
    {
        example();

        cout << "Doing other work...\n";

        // Some event happens (timer expires, network reply arrives, etc.)
        saved.resume();

        cout << "Done\n";
    }
}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    cout << "main start\n";

    // normalFunction::printNumbers();

    // co_return
    //  coroutineExample1::hello();

    // co_yield
    //  coroutineExample2::test();

    // Notice "After co_await" is never printed because we suspended the coroutine but never resumed it.
    // coroutineExample3::example();

    // Resuming co_await
    coroutineExample4::test();

    cout << "main end\n";

    return 0;
}
