
#include <iostream>
#include <new>

// Overloading operator new, for Unit testing, forcing behavior
void *operator new(std::size_t size)
{
    std::cout << "Forced failure in global operator new\n";
    throw std::bad_alloc(); // simulate memory failure
}

int main()
{
    try
    {
        int *p = new int;

        // int *p = new (std::nothrow) int;
        // if (!p)
        // {
        //     std::cout << "p is nullptr \n";
        //     return 0;
        // }

        delete p;
        p = nullptr;
    }
    catch (const std::bad_alloc &)
    {
        std::cout << "Caught bad_alloc (forced)!\n";
    }

    return 0;
}
