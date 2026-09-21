SMART POINTERS
    When a smart pointer goes out of scope or is reset, 
    it can automatically free the resource it holds.

    Your default smart pointer should be unique_ptr. 
    Only use shared_ptr when you really need to share the resource.

    Never assign the result of a resource allocation to a dumb pointer. 
    Whatever resource allocation method you use, always immediately store the resource pointer in a smart pointer, 
    either unique_ptr or shared_ptr, or use other RAII classes. 
    RAII stands for Resource Acquisition Is Initialization. 
    An RAII class takes ownership of a certain resource and handles its deallocation at the right time. 
    It’s a design technique.

unique_ptr
Creating unique_ptrs
    Always use make_unique() to create a unique_ptr.