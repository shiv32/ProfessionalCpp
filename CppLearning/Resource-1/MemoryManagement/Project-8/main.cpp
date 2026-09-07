#include <iostream>

using namespace std;

namespace example1
{
    void test_1()
    {
        // an array of ints on the heap
        int *myArray = new int[8];

        // setting the value in position 2
        myArray[2] = 33;

        // With pointer arithmetic
        *(myArray + 2) = 33;
    }

    wchar_t *toCaps(const wchar_t *inString)
    {
        if (!inString)
            return nullptr;

        std::size_t len = std::wcslen(inString);

        wchar_t *result = new wchar_t[len + 1];

        for (std::size_t i = 0; i < len; ++i)
            result[i] = std::towupper(inString[i]);

        result[len] = L'\0';

        return result;
    }

    void test_2()
    {
        // To tell the compiler that a string literal is a wide-string literal, prefix it with an L
        const wchar_t *myString = L"Hello, World";

        wchar_t *caps = toCaps(myString + 7);

        wprintf(L"%ls\n", caps);

        delete[] caps;
    }

    // Another useful application of pointer arithmetic involves subtraction.
}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    example1::test_2();

    return EXIT_SUCCESS;
}