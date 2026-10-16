Move Semantics
    The standard smart pointers, shared_ptr, unique_ptr, and weak_ptr all support move semantics to make them efficient.
    This means it is very efficient to return such a smart pointer from a function.

enable_shared_from_this
    The std::enable_shared_from_this mixin class allows a method on an object to safely return a shared_ptr or weak_ptr to itself.

The Old Deprecated/Removed auto_ptr
    C++17 finally removed it entirely.
    Do not use the old auto_ptr smart pointer anymore. 
    Instead, use unique_ptr or shared_ptr!
   