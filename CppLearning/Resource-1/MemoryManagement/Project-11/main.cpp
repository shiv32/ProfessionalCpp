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
        auto mySimpleSmartPtr = make_shared<Simple>();

        // Smart pointers can still be dereferenced (using * or ->) just like standard pointers.
        mySimpleSmartPtr->go();
        (*mySimpleSmartPtr).go();

        processData(mySimpleSmartPtr.get());

        mySimpleSmartPtr.reset(); // Free resource and set to nullptr

        cout << "isNull : " << isNull(mySimpleSmartPtr) << endl;

        mySimpleSmartPtr.reset(new Simple()); // Free resource and set to a new Simple instance

        cout << "isNull : " << isNull(mySimpleSmartPtr) << endl;
    }

    /*
        Custom Deleters
        By default, shared_ptr uses the standard new and delete operators to allocate and deallocate memory.
        You can change this behavior as follows
    */

    int *malloc_int(int value)
    {
        int *p = (int *)malloc(sizeof(int)); // in C++ you should never use malloc(), but new instead.
        *p = value;
        return p;
    }

    void test_2()
    {
        /*
            you don’t have to specify the type of the custom deleter as a template type parameter,
            so this makes it much easier than a custom deleter with unique_ptr.
        */
        shared_ptr<int> myIntSmartPtr(malloc_int(42), free);

        cout << "myIntSmartPtr : " << *myIntSmartPtr << endl;
    }

    /*
        Note that C++ has proper object-oriented classes to work with files.
        Those classes already automatically close their files.
        This example using the old C functions fopen() and fclose() is just to give a demonstration of what shared_ptrs
        can be used for besides pure memory.
    */
    void CloseFile(FILE *filePtr)
    {
        if (filePtr == nullptr)
            return;

        fclose(filePtr);

        cout << "File closed." << endl;
    }

    void test_3()
    {
        FILE *f = fopen("data.txt", "w");

        shared_ptr<FILE> filePtr(f, CloseFile); // shared_ptr to store a file pointer

        if (filePtr == nullptr)
        {
            cerr << "Error opening file." << endl;
        }
        else
        {
            cout << "File opened." << endl;
            // Use filePtr
        }
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