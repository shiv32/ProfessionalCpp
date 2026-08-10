#include <cstdlib>
#include <iostream>
#include <new>

using namespace std;

namespace example1
{
    class Simple
    {
    public:
        Simple() { cout << "Simple constructor called!" << endl; }
        ~Simple() { cout << "Simple destructor called!" << endl; }
    };

    void test_1()
    {
        // If you allocate an array of four Simple objects, the Simple constructor is called four times.
        Simple *mySimpleArray = new Simple[4];

        // Use mySimpleArray

        delete[] mySimpleArray; // Deleting Arrays
        mySimpleArray = nullptr;
    }

    void test_2()
    {
        const size_t size = 4;

        Simple **mySimplePtrArray = new Simple *[size];

        // Allocate an object for each pointer.
        for (size_t i = 0; i < size; i++)
        {
            mySimplePtrArray[i] = new Simple();
        }

        // Use mySimplePtrArray

        // Delete each allocated object.
        for (size_t i = 0; i < size; i++)
        {
            delete mySimplePtrArray[i];
        }

        // Delete the array itself.
        delete[] mySimplePtrArray;
        mySimplePtrArray = nullptr;
    }

}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    //example1::test_1();
    example1::test_2();

    return EXIT_SUCCESS;
}