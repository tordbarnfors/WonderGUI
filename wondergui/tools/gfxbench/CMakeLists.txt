set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	gfxbench.cpp
	gfxbench.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(gfxbench
	${SOURCES}
)

target_include_directories(gfxbench PRIVATE
	${WAPP_INCDIRS}
)

target_working_directory(gfxbench ${WG_ROOT} )

target_link_libraries(gfxbench PRIVATE
	${WAPPLIB}
)


