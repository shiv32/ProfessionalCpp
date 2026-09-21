#include <iostream>
#include <memory>

using namespace std;

namespace example1
{
    //--------------------------------------------------
    struct Simple
    {
        void go()
        {
        }
    };

    struct Data
    {
    };

    Data data()
    {
        return Data{};
    }

    struct Bar
    {
        Bar(Data d)
        {
            // use d
        }
    };

    void foo(std::unique_ptr<Simple> simple, std::unique_ptr<Bar> bar)
    {
    }
    //-----------------------------------------------------

    void leaky()
    {
        Simple *mySimplePtr = new Simple(); // BUG! Memory is never released!
        mySimplePtr->go();
    }

    void couldBeLeaky()
    {
        Simple *mySimplePtr = new Simple();
        mySimplePtr->go(); // If the go() method throws an exception, the call to delete is never executed, causing a memory leak.
        delete mySimplePtr;
    }

    void notLeaky()
    {
        auto mySimpleSmartPtr = make_unique<Simple>();

        // If your compiler does not yet support make_unique()
        // unique_ptr<Simple> mySimpleSmartPtr(new Simple());

        mySimpleSmartPtr->go();
    }

    void test_1()
    {
        /*
            If the constructor of Simple or Bar, or the data() function, throws an exception,
            depending on your compiler optimizations,
            it was very possible that either a Simple or a Bar object would be leaked.
        */
        foo(unique_ptr<Simple>(new Simple()), unique_ptr<Bar>(new Bar(data())));

        // With make_unique(), nothing would leak
        foo(make_unique<Simple>(), make_unique<Bar>(data()));
    }
}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    example1::test_1();

    return EXIT_SUCCESS;
}