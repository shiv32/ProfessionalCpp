#include <iostream>
#include <memory>

using namespace std;

namespace example1
{
    struct Simple
    {
        void go()
        {
            cout << "go.." << endl;
        }
    };

    void processData(Simple *simple)
    {
        /* Use the simple pointer… */
        cout << "processData.." << endl;
    }

    template <typename T>
    bool isNull(const T &ptr)
    {
        return ptr == nullptr;
    }

    void test_1()
    {
        auto mySimpleSmartPtr = make_unique<Simple>();

        // Smart pointers can still be dereferenced (using * or ->) just like standard pointers.
        mySimpleSmartPtr->go();
        (*mySimpleSmartPtr).go();

        processData(mySimpleSmartPtr.get());

        mySimpleSmartPtr.reset(); // Free resource and set to nullptr

        cout << "isNull : " << isNull(mySimpleSmartPtr) << endl;

        mySimpleSmartPtr.reset(new Simple()); // Free resource and set to a new Simple instance

        cout << "isNull : " << isNull(mySimpleSmartPtr) << endl;

        /*
         You can disconnect the underlying pointer from a unique_ptr with release().
         The release() method returns the underlying pointer to the resource and then sets the smart pointer to nullptr.
        */
        Simple *simple = mySimpleSmartPtr.release(); // Release ownership

        cout << "isNull : " << isNull(mySimpleSmartPtr) << endl;

        // Use the simple pointer…

        delete simple;
        simple = nullptr;
    }

    class Foo
    {

    public:
        // Because a unique_ptr represents unique ownership, it cannot be copied!
        Foo(unique_ptr<int> data) : mData(move(data)) // explicitly move ownership
        {
        }

        int getValue()
        {
            return *mData;
        }

    private:
        unique_ptr<int> mData;
    };

    void test_2()
    {
        auto myIntSmartPtr = make_unique<int>(42);

        Foo f(move(myIntSmartPtr)); // explicitly move ownership

        cout << "getValue : " << f.getValue() << endl;
    }

    void test_3()
    {
        // A unique_ptr is suitable to store a dynamically allocated old C-style array.
        // creates a unique_ptr that holds a dynamically allocated C-style array of ten integers
        auto myVariableSizedArray = make_unique<int[]>(10);

        // it’s recommended to use a Standard Library container instead, such as std::array or std::vector.

        // Put data
        for (int i = 0; i < 10; ++i)
            myVariableSizedArray[i] = (i + 1) * 10;

        // Display data
        for (int i = 0; i < 10; ++i)
            std::cout << myVariableSizedArray[i] << ' ';

        cout << endl;

        // you do not need delete[]—unique_ptr<int[]> automatically releases the array.
    }

    /*
        Custom Deleters
        By default, unique_ptr uses the standard new and delete operators to allocate and deallocate memory.
        You can change this behavior as follows
    */

    int *malloc_int(int value)
    {
        int *p = (int *)malloc(sizeof(int)); // in C++ you should never use malloc(), but new instead.
        *p = value;
        return p;
    }

    void test_4()
    {
        // this feature of unique_ptr is available because it is useful to manage other resources instead of just memory.
        // it can be used to automatically close a file or network socket or anything when the unique_ptr goes out of scope.
        // specify the type of your custom deleter as a template type parameter
        // decltype(free) is used which returns the type of free()
        // The template type parameter should be the type of a pointer to a function
        unique_ptr<int, decltype(free) *> myIntSmartPtr(malloc_int(42), free);

        cout << "myIntSmartPtr : " << *myIntSmartPtr << endl;
    }

}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    // example1::test_1();
    // example1::test_2();
    // example1::test_3();
    example1::test_4();

    return EXIT_SUCCESS;
}