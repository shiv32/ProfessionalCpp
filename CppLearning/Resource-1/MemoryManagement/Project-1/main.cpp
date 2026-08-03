#include <iostream>

using namespace std;

namespace example1
{
    void test1()
    {
        int value = 10;
        int *ptr1 = &value;

        std::cout << "&value(stack) : " << &value << "\n";
        std::cout << "value(value at stack) : " << value << "\n";

        std::cout << "&ptr1(stack) : " << &ptr1 << "\n";
        std::cout << "ptr1(stack) : " << ptr1 << "\n";
        std::cout << "*ptr1(value at stack) : " << *ptr1 << "\n";

        int **handlee = &ptr1;

        std::cout << "handlee(stack) : " << handlee << "\n";
        std::cout << "*handlee(stack) : " << *handlee << "\n";
        std::cout << "**handlee(value at stack) : " << **handlee << "\n";

        std::cout << "-----------------------------------------\n";

        int i = 7;

        std::cout << "&i(stack) : " << &i << "\n";
        std::cout << "i(value at stack) : " << i << "\n";

        int *ptr = nullptr; // int* ptr = new int;
        // ptr = new int;
        ptr = new int(45);

        std::cout << "&ptr(stack) : " << &ptr << "\n";
        std::cout << "ptr(heap) : " << ptr << "\n";
        std::cout << "*ptr(value at heap) : " << *ptr << "\n";

        int **handle = nullptr;
        handle = new int *;
        *handle = new int(6);

        std::cout << "&handle(stack) : " << &handle << "\n";
        std::cout << "handle(heap)  : " << handle << "\n";
        std::cout << "*handle(heap) : " << *handle << "\n";
        std::cout << "**handle(value at heap) : " << **handle << "\n";
    }
}

namespace example2
{
    void test1()
    {
        int i = 7; // i is a so-called automatic variable allocated on the stack.

        int *ptr = nullptr; // variable ptr on the stack initialized with nullptr
        ptr = new int;      // When you use the new keyword, memory is allocated on the heap.

        // above code in one-liner
        int *ptr2 = new int; // When you use the new keyword, memory is allocated on the heap.

        // pointers can exist both on the stack and on the heap
        int **handle = nullptr;
        handle = new int *; // stack (handle)
        *handle = new int;  // heap (*handle)
    }

    void leaky()
    {
        new int; // BUG! Orphans/leaks memory!
        cout << "I just leaked an int!" << endl;
    }

    void test2()
    {
        int *ptr = new int;
        delete ptr;
        ptr = nullptr;
    }
}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    example1::test1();

    return EXIT_SUCCESS;
}