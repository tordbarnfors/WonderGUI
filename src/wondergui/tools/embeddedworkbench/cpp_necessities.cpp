

#include <wax_memheap.h>
#include <wax_c_memheap.h>
#include <cstddef>

#include "cpp_necessities.h"


static wax::MemHeap* cpp_memHeap;

void init_cpp_necessities(wax_memHeap heap)
{
  cpp_memHeap = reinterpret_cast<wax::MemHeap*>(heap);
}

void exit_cpp_necessities()
{
}


namespace std
{
    void __throw_bad_array_new_length(){ while(1); }

	void  __throw_bad_function_call(){ while(1); }

	void __throw_bad_alloc() { while(1); }

	void __throw_length_error( char const*e ) { while(1); }
};


// extern "C" void __cxa_pure_virtual() { while (1); }

// void * malloc(size_t size)
// {
//     void *p;
//     p =  cpp_memHeap->malloc(size);
//     if(!p)
//     {
// 		// Do something...
//     }
//     return p;
// }

// void free(void *p)
// {
//     cpp_memHeap->free(p);
// }

/*

void * operator new(size_t size)
{
    void *p;
    p =  cpp_memHeap->malloc(size);
    if(!p)
    {
		// Do something...
    }
    return p;
}

void operator delete(void *p)
{
    cpp_memHeap->free(p);
}

void * operator new[](size_t size)
{
    void *p;
    p =  cpp_memHeap->malloc(size);
    if(!p)
     {
        // Do something...
    }
    return p;
}

void operator delete[](void *p)
{
    cpp_memHeap->free(p);
}

*/
