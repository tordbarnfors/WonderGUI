set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	softkernelgenerator.cpp
	softkernelgenerator.h
	kerneldb.cpp
	kerneldb.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(softkernelgenerator
	${SOURCES}
)

target_include_directories(softkernelgenerator PRIVATE 
	${WAPP_INCDIRS}
	${WGSOFTGFX_INCDIRS}		# For wg_softformat.h
)

target_working_directory(softkernelgenerator ${WG_ROOT} )

target_link_libraries(softkernelgenerator PRIVATE
	${WAPPLIB}
)


