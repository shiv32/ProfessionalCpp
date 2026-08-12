#include <iostream>

using namespace std;

namespace example1
{
    void test_1()
    {
        // array declared on the stack, zero-initialized, and accessed with some test code
        char board[3][3] = {}; // think of the first subscript as the x-axis and the second as the y-axis.

        // Test code
        board[0][0] = 'X'; // X puts marker in position (0,0).
        board[2][1] = 'O'; // O puts marker in position (2,1).
    }

    void test_2()
    {
        //This code doesn’t compile because heap-based arrays don’t work like stack-based arrays.

        // const int i{3},j{3};
        // char** board = new char[i][j]; // BUG! Doesn't compile
    }

    //The following function properly allocates memory for a two-dimensional array in heap
    char **allocateCharacterBoard(size_t xDimension, size_t yDimension)
    {
        char **myArray = new char *[xDimension]; // Allocate first dimension

        for (size_t i = 0; i < xDimension; i++)
        {
            myArray[i] = new char[yDimension]; // Allocate ith subarray
        }

        return myArray;
    }

    //Release an array should mirror the code to allocate it in heap, as in the following function
    void releaseCharacterBoard(char **myArray, size_t xDimension)
    {
        for (size_t i = 0; i < xDimension; i++)
        {
            delete[] myArray[i]; // Delete ith subarray
        }

        delete[] myArray; // Delete first dimension
    }

    void test_3()
    {
        char **myArray = allocateCharacterBoard(2, 2);

        // do work with myArray

        releaseCharacterBoard(myArray, 2);
    }

}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    // example1::test_1();
    example1::test_3();

    return EXIT_SUCCESS;
}