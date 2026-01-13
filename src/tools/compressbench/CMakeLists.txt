set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	compressbench.cpp
	compressbench.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(compressbench
	${SOURCES}
)

target_include_directories(compressbench PRIVATE 
	${WAPP_INCDIRS}
)

target_working_directory(compressbench ${WG_ROOT} )

target_link_libraries(compressbench PRIVATE
	${WAPPLIB}
)


