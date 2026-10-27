set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	helloworld.cpp
	helloworld.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(helloworld
	${SOURCES}
)

target_include_directories(helloworld PRIVATE 
	${WAPP_INCDIRS}
)

target_working_directory(helloworld ${WG_ROOT} )

target_link_libraries(helloworld PRIVATE
	${WAPPLIB}
)


