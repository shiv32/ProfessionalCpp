#include <iostream>
#include <memory>
#include <cstring>

using namespace std;

namespace example1
{
    char *getMoreData()
    {
        static int count = 0;

        if (count >= 3)
        {
            return nullptr;
        }

        const char *chunks[] = {
            "Hello ",
            "World ",
            "C++17"};

        const char *chunk = chunks[count++];

        char *result = new char[std::strlen(chunk) + 1];
        std::strcpy(result, chunk);

        return result;
    }

    /*
        Underallocating Strings

        It expects the destination buffer to be big enough.
    */
    void test_1()
    {
        char buffer[1024] = {0}; // Allocate a whole bunch of memory.
        // Use C++-style strings, which handle the memory associated with concatenation on your behalf.

        while (true)
        {
            char *nextChunk = getMoreData();

            if (nextChunk == nullptr)
            {
                break;
            }
            else
            {
                strcat(buffer, nextChunk); // BUG! No guarantees against buffer overrun!
                delete[] nextChunk;
            }
        }

        cout << buffer << endl;
    }

    void fillWithM(char *inStr)
    {
        int i = 0;

        while (inStr[i] != '\0')
        {
            inStr[i] = 'm';
            i++;
        }
    }

    // Accessing Out-of-Bounds Memory
    void test_2()
    {
        char str[] = "Hello World";

        /*
            The program might crash, print unexpected characters, or appear to work.
            There is no guaranteed result.

            Demonstrate it with AddressSanitizer

            g++ -std=c++17 -g -O0 -fsanitize=address -fno-omit-frame-pointer main.cpp -o main && ./main
        */
        // char str[5] = {'H', 'e', 'l', 'l', 'o'}; // No '\0'

        // char str[6] = {'H', 'e', 'l', 'l', 'o'}; // has '\0'

        /*
            std::string str = "Hello, World!";

            // Both methods return a null-terminated const char*
            const char* ptr1 = str.c_str();
            const char* ptr2 = str.data();

            std::cout << ptr1 << std::endl;
        */
        // string str{"hello"};
        // fillWithM(str.data());

        fillWithM(str);

        std::cout << str << '\n';
    }

    struct Simple
    {
        Simple()
        {
            cout << "Simple constructor called!" << endl;

            mIntPtr = new int();
        }

        ~Simple()
        {
            cout << "Simple destructor called!" << endl;

            delete mIntPtr;
        }

        void setValue(int value) { *mIntPtr = value; }

        int *mIntPtr;
    };

    void doSomething(Simple *&outSimplePtr)
    {
        outSimplePtr = new Simple(); // BUG! Doesn't delete the original.
    }

    /*
        Memory Leaks

        Compile & run with LeakSanitizer

            g++ -std=c++17 -g -O0 -fsanitize=leak main.cpp -o main && ./main

        AddressSanitizer + LeakSanitizer

            g++ -std=c++17 -g -O0 -fsanitize=address -fno-omit-frame-pointer main.cpp -o main && ./main
    */
    void test_3()
    {
        Simple *simplePtr = new Simple(); // Allocate a Simple object.

        doSomething(simplePtr);

        delete simplePtr; // Only cleans up the second object.

        /*
            Keep in mind that this code is only for demonstration purposes!
            In production-quality code, you should make mIntPtr and simplePtr unique_ptrs,
            and make outSimplePtr a reference to a unique_ptr.
        */
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