#include <iostream>
#include <memory>

using namespace std;

namespace example1
{
    struct Simple
    {
        Simple()
        {
            cout << "Simple constructor called!" << endl;
        }

        ~Simple()
        {
            cout << "Simple destructor called!" << endl;
        }
    };

    unique_ptr<Simple> create()
    {
        auto ptr = make_unique<Simple>();

        // Do something with ptr…

        return ptr;
    }

    void test_1()
    {
        unique_ptr<Simple> mySmartPtr1 = create();
        auto mySmartPtr2 = create();
    }

    /*
    Completely wrong implementation of the getPointer() method

    struct Foo
    {
        shared_ptr<Foo> getPointer()
        {
            return shared_ptr<Foo>(this);
        }
    };

    */

    struct Foo : public enable_shared_from_this<Foo>
    {
        Foo()
        {
            cout << "Foo constructor called!" << endl;
        }

        ~Foo()
        {
            cout << "Foo destructor called!" << endl;
        }

        shared_ptr<Foo> getPointer()
        {
            return shared_from_this();
        }
    };

    void test_2()
    {
        auto ptr1 = make_shared<Foo>();
        auto ptr2 = ptr1->getPointer();
    }

}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    // example1::test_1();
    example1::test_2();

    return EXIT_SUCCESS;
}