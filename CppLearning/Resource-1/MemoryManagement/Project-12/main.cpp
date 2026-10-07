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

    void doubleDelete()
    {
        Simple *mySimple = new Simple();
        shared_ptr<Simple> smartPtr1(mySimple);
        shared_ptr<Simple> smartPtr2(mySimple);
    }

    void noDoubleDelete()
    {
        auto smartPtr1 = make_shared<Simple>();
        shared_ptr<Simple> smartPtr2(smartPtr1); // make a copy
    }

    void test_1()
    {
        doubleDelete(); // o/p: free(): double free detected in tcache 2
    }

    void test_2()
    {
        noDoubleDelete();
    }

    struct Foo
    {
        Foo(int value) : mData(value)
        {
            cout << "Foo constructor called: " << mData << endl;
        }

        int mData;
    };

    void test_3()
    {
        auto foo = make_shared<Foo>(42);

        /*
        aliasing:
            Points to: foo->mData
            Owns: the whole Foo object indirectly
            *aliasing → reads/writes mData
            foo can be destroyed, but Foo stays alive while aliasing exists.

            Key idea: shared_ptr<int> pointing to a member, but ownership remains with Foo.
                      aliasing stores &foo->mData, but its ownership is still tied to the Foo object.
        */
        auto aliasing = shared_ptr<int>(foo, &foo->mData);

        /*
            Owned pointer: foo → owns the Foo object
            Stored pointer: aliasing.get() → points to foo->mData
        */

        *aliasing = 100;        // changes foo->mData
        
        std::cout << *aliasing<<endl; // 100
        std::cout << foo->mData<<endl; // 100

        std::cout << aliasing.get()<<endl; //0x559bef70f030
        std::cout << &foo->mData<<endl; //0x559bef70f030

        //The Foo object is only destroyed when both shared_ptrs (foo and aliasing) are destroyed.
    }

}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    // example1::test_1();
    // example1::test_2();
    example1::test_3();

    return EXIT_SUCCESS;
}