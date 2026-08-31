#include <iostream>

using namespace std;

namespace example1
{
    // The caller of this function can pass a stack-based or heap-based array.
    void printArray(int *theArray, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            cout << theArray[i] << " ";
        }

        cout << endl;
    }

    // A function template,can be used to let the compiler deduce the size of the stack-based array automatically.
    template <size_t N>
    void printStackArray(int (&theArray)[N])
    {
        for (size_t i = 0; i < N; i++)
        {
            cout << theArray[i] << " ";
        }

        cout << endl;
    }

    void test_1()
    {
        // The following code creates a zero-initialized array on the stack,
        // and uses a pointer to access it:
        int myIntArray[10] = {};
        int *myIntPtr = myIntArray;

        // Access the array through the pointer.
        myIntPtr[4] = 5;

        printArray(myIntPtr, 10);

        // The ability to refer to a stack-based array through a pointer is useful when passing arrays into functions.
    }

    // The caller of this function can pass a stack-based or heap-based array.
    void doubleInts(int *theArray, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            theArray[i] *= 2;
        }
    }

    // The following implementation of doubleInts_2() changes the original array
    // even though the parameter is an array, not a pointer:
    void doubleInts_2(int theArray[], size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            theArray[i] *= 2;
        }
    }

    /*
        The following three versions are identical:

        void doubleInts(int* theArray, size_t size);
        void doubleInts(int theArray[], size_t size);
        void doubleInts(int theArray[2], size_t size);
    */

    // the following doubleIntsStack() accepts only stack-based arrays of size 4.
    // This does not work for heap-based arrays.
    void doubleIntsStack(int (&theArray)[4])
    {
        for (size_t i = 0; i < 4; i++)
        {
            theArray[i] *= 2;
        }
    }

    // A function template,can be used to let the compiler deduce the size of the stack-based array automatically.
    template <size_t N>
    void doubleIntsStack_2(int (&theArray)[N])
    {
        for (size_t i = 0; i < N; i++)
        {
            theArray[i] *= 2;
        }
    }

    // heap-based array
    void test_2()
    {
        size_t arrSize = 4;
        int *heapArray = new int[arrSize]{1, 5, 3, 4};
        doubleInts(heapArray, arrSize); // heap-based array
        printArray(heapArray, arrSize);
        delete[] heapArray;
        heapArray = nullptr;
    }

    // stack-based array
    void test_3()
    {
        size_t arrSize{};
        int stackArray[] = {5, 7, 9, 11};
        arrSize = std::size(stackArray); // Since C++17, requires <array>
        // arrSize = sizeof(stackArray) / sizeof(stackArray[0]); // Pre-C++17
        doubleInts(stackArray, arrSize); // stack-based array
        printArray(stackArray, arrSize);

        // compiler treats an array as a pointer when it is passed to a function.
    }

    // address of the first element
    void test_4()
    {
        size_t arrSize{};
        int stackArray[] = {5, 7, 9, 11};
        arrSize = std::size(stackArray);     // Since C++17, requires <array>
        doubleInts(&stackArray[0], arrSize); // address of the first element
        printArray(stackArray, arrSize);
    }

    /*
        To summarize, arrays declared using array syntax can be accessed through a pointer.
        When an array is passed to a function, it is always passed as a pointer.
    */

    void test_5()
    {
        /*
            A pointer by itself is meaningless.
            It may point to random memory, a single object, or an array.
        */

        //  The pointer ptr is a valid pointer,
        //  but it is not an array.
        int *ptr = new int;
    }

    void test_6()
    {
        int stackArray[] = {5, 7, 9, 11};
        doubleIntsStack_2(stackArray); // stack-based array
        printStackArray(stackArray);
    }
}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    example1::test_1();
    // example1::test_2();
    // example1::test_3();
    // example1::test_4();
    // example1::test_6();

    return EXIT_SUCCESS;
}