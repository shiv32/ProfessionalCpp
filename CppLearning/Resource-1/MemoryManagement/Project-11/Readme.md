shared_ptr
    The Standard Library provides std::shared_ptr which is a smart pointer with 
    shared ownership semantics using reference counting. The standard shared_ptr is thread-safe, 
    but this does not mean that the pointed-to resource is thread-safe!
    
    Always use make_shared() to create a shared_ptr.

Casting a shared_ptr
    const_pointer_cast()
    dynamic_pointer_cast()
    static_pointer_cast()
    reinterpret_pointer_cast() (C++17)