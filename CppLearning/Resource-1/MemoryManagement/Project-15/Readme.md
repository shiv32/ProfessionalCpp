COMMON MEMORY PITFALLS
    there are several common categories of problems and some tools you can use to detect and resolve them.

    Underallocating Strings
        this arises when the programmer fails to allocate an extra character for the trailing '\0' sentinel.

    Accessing Out-of-Bounds Memory
        Bugs that result in writing to memory past the end of an array are often called buffer overflow errors.
        These bugs have been exploited by several high-profile malware programs such as viruses and worms.

        Avoid using old C-style strings and arrays that offer no protection whatsoever. 
        Instead, use modern and safe constructs like C++ strings and vectors that manage all their memory for you.

    Memory Leaks
        Memory leaks occur when you allocate memory and neglect to release it.

Finding and Fixing Memory Leaks in Windows with Visual C++
    Microsoft Visual C++, its debug library has built-in support for memory leak detection.
    There is a free version of Microsoft Visual C++ available, called the Community Edition.

Finding and Fixing Memory Leaks in Linux with Valgrind
    Valgrind is an example of a free open-source tool for Linux.

    It is strongly recommended to use smart pointers as often as possible to avoid memory leaks.

Double-Deleting and Invalid Pointers
    Many memory leak-detection programs, such as Microsoft Visual C++ and Valgrind, 
    are capable of detecting double deletion and use of released objects.

SUMMARY
    you should try to avoid using old C-style constructs and functions as much as possible, 
    and use the safe C++ alternatives.

