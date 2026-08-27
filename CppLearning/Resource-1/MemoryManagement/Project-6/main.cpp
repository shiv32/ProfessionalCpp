#include <iostream>

using namespace std;

namespace example1
{
    void test_1()
    {
        /*
            pointer is just a memory address, you could theoretically change that address manually,
            even doing something as scary as the following line of code.

            This line builds a pointer to the memory address 7,
            which is likely to be random garbage or memory used elsewhere in the application.
        */

        char *scaryPointer = (char *)7;
    }

    class Document
    {
    public:
        Document() {};
    };

    Document *getDocument()
    {
        return new Document();
    }

    void test_2()
    {
        Document *documentPtr = getDocument();
        // The compiler will let you easily cast any pointer type to any other pointer type using a C-style cast.
        char *myCharPtr = (char *)documentPtr;

        // A static cast offers a bit more safety.
        // The compiler refuses to perform a static cast on pointers to unrelated data types:
        // char* myCharPtr = static_cast<char*>(documentPtr);   // BUG! Won't compile

        delete documentPtr;//no memory leak
        documentPtr = nullptr;
        
        myCharPtr = nullptr;//no remaining dangling pointer.
    }

}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    example1::test_1();

    return EXIT_SUCCESS;
}