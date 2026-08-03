#include <cstdlib>
#include <iostream>
#include <new>

using namespace std;

namespace example1
{
    void test_1()
    {
        int myArray[5];
    }

    void test_2()
    {
        int *myArrayPtr = new int[5];

        // Do you work

        delete[] myArrayPtr;
        myArrayPtr = nullptr;
    }
}

namespace example2
{
    class Document
    {
    public:
        Document() = default;
        ~Document() = default;
    };

    size_t askUserForNumberOfDocuments()
    {
        return 2;
    }

    Document *createDocArray()
    {
        size_t numDocs = askUserForNumberOfDocuments();
        Document *docArray = new Document[numDocs];
        return docArray;
    }

    void test_1()
    {
        Document *docArray = createDocArray();

        // Do you work

        delete[] docArray;
        docArray = nullptr;
    }
}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    // example1::test_1();
    // example1::test_2();

    example2::test_1();

    return EXIT_SUCCESS;
}