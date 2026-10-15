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

    void useResource(weak_ptr<Simple> &weakSimple)
    {
        auto resource = weakSimple.lock();

        if (resource)
        {
            cout << "Resource still alive." << endl;
        }
        else
        {
            cout << "Resource has been freed!" << endl;
        }
    }

    void test_1()
    {
        auto sharedSimple = make_shared<Simple>();
        weak_ptr<Simple> weakSimple(sharedSimple);

        // Try to use the weak_ptr.
        useResource(weakSimple);

        /*
            Reset the shared_ptr.
            Since there is only 1 shared_ptr to the Simple resource, this will
            free the resource, even though there is still a weak_ptr alive.
        */
        sharedSimple.reset();

        // Try to use the weak_ptr a second time.
        useResource(weakSimple);
    }

}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    example1::test_1();

    return EXIT_SUCCESS;
}