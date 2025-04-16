

set(SOURCES
	wg_gearbase.cpp
	wg_gearbase.h
	wg_blob.cpp
	wg_blob.h
	wg_chain.cpp
	wg_chain.h
	wg_componentptr.h
	wg_dataset.cpp
	wg_dataset.h
	wg_dynamicbuffer.h
	wg_memheap.cpp
	wg_memheap.h
	wg_mempool.cpp
	wg_mempool.h
	wg_memstack.cpp
	wg_memstack.h
	wg_object.cpp
	wg_object.h
	wg_pointers.h
	wg_strongptr.h
	wg_gearenumextras.cpp
	wg_gearenumextras.h
	wg_geartypes.h
	wg_weakptr.h

	c-api/wg_c_gearbase.cpp
	c-api/wg_c_gearbase.h
	c-api/wg_c_blob.cpp
	c-api/wg_c_blob.h
	c-api/wg_c_geartypes.cpp
	c-api/wg_c_geartypes.h
	c-api/wg_c_memheap.cpp
	c-api/wg_c_memheap.h
	c-api/wg_c_object.cpp
	c-api/wg_c_object.h
	c-api/wondergear_c.h

	wondergear.h
)


set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/c-api
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wondergear STATIC ${SOURCES} )
target_include_directories(wondergear PRIVATE ${INCDIRS})


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WONDERGEAR_SOURCES ${SOURCES} PARENT_SCOPE)
set(WONDERGEAR_INCDIRS ${INCDIRS} PARENT_SCOPE)

