#include <cstdlib>
#include <iostream>
#include <new>

using namespace std;

namespace example1
{
    class Foo
    {
    public:
        Foo()
        {
            cout << "Foo ctor." << endl;
        }

        ~Foo()
        {
            cout << "Foo dtor" << endl;
        }
    };

    void test1()
    {
        /*
            The malloc() function only sets aside a piece of memory of a certain size.
            It doesn’t know about or care about objects.
        */
        Foo *myFoo = (Foo *)malloc(sizeof(Foo));

        free(myFoo); // With free(), the object’s destructor is not called.
        myFoo = nullptr;
    }

    void test2()
    {
        /*
            The call to new allocates the appropriate size of memory and also calls an appropriate constructor
            to construct the object.
        */
        Foo *myOtherFoo = new Foo();

        delete myOtherFoo; // With delete, the destructor is called and the object is properly cleaned up.
        myOtherFoo = nullptr;
    }
}

// Overloading operator new, for Unit testing, forcing behavior
void *operator new(std::size_t size)
{
    std::cout << "Forced failure in global operator new\n";
    throw std::bad_alloc(); // simulate memory failure
}

namespace example2
{
    void test1()
    {
        try
        {
            int *p = new int; //standard version of new

            delete p;
            p = nullptr;

            cout << "try end" << endl;
        }
        catch (const std::bad_alloc &)
        {
            std::cout << "Caught bad_alloc (forced)!\n";
        }
    }

    void test2()
    {
        try
        {
            int *p = new (std::nothrow) int; //it’s suggested that you use the standard version of new

            if (!p)
            {
                std::cout << "p is nullptr \n";
            }
            else
            {
                delete p;
                p = nullptr;
            }
        }
        catch (const std::bad_alloc &)
        {
            std::cout << "Caught bad_alloc (forced)!\n";
        }
    }
}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    // example1::test1();
    //  example1::test2();

    example2::test1();
    // example2::test2();

    return EXIT_SUCCESS;
}