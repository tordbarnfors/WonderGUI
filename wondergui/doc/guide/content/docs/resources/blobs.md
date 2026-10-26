---
title: 'Blobs'
weight: 10
---


# Blobs

A *Blob* is a simple container that can be used to wrap any data into an *Object* with smart pointers and reference counters. They can be created with empty memory or pointing to existing data in memory. When they are destroyed, the memory is typically returned.



## Creation

An empty Blob can be created by just specifying the size in bytes of the memory area you need:

```c++
auto pMyBlob = Blob::create( 1024 );
```

The memory will be released when the Blob is destroyed.

You can also turn an existing object (non-WonderGUI object that is) or chunk of memory into a Blob by providing a pointer, size and destructor-function:

```c++
char * pMyData = new char[1024];
auto pMyBlob = create( pMyData, 1024, [pMyData](){delete [] pMyData;} );
```

The size parameter is not necessary for the blob and can be omitted, it is just what will be returned by the size() method. If size is not specified the size() method will return 0.

The destructor is run when the Blob is destroyed and is responsible for cleaning up. It doesn't necessarily have to delete anything. In some cases you might just want to make the data available in a reference counted way and be noted when it is not used anymore.



## Accessing the data

To access the memory of the blob you just call the data() or begin() methods, they are identical. The methods size() and end() can be used to get size and a end-pointer respectively, but be aware that in cases where the size is unknown size() returns 0 and end() will be identical to begin().



