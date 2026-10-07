The Need for Reference Counting
    As a general concept, 
    reference counting is a technique for keeping track of the number of instances of a class or particular object in use.
    unique_ptr is not reference counted.

Aliasing
    A shared_ptr support so-called aliasing. 
    This allows a shared_ptr to share ownership over a pointer (owned pointer) with another shared_ptr, 
    but pointing to a different object (stored pointer).