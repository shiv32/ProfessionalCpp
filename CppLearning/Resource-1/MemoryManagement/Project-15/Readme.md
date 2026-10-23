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