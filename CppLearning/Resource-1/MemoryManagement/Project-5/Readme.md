Multi-dimensional Arrays
    Multi-dimensional arrays extend the notion of indexed values to multiple indices.

Multi-dimensional Stack Arrays
    dimensions higher than three(x,y,z) tend to be difficult to conceptualize and are rarely used.

Multi-dimensional Heap Arrays
    If you need to determine the dimensions of a multi-dimensional array at run time, 
    you can use a heap-based array.

    use vector<T> for a one-dimensional dynamic array, 
    use vector<vector<T>> for a two-dimensional dynamic array,
    
    to work with two-dimensional data with equally long rows, 
    you should consider writing (or reusing of course) a Matrix<T> or Table<T> class template 
    (helper classes that provide an easier to use interface)
    which internally might use a vector<vector<T>> data structure.

    Use C++ Standard Library containers such as std::array, std::vector, and so on, instead of C-style arrays!