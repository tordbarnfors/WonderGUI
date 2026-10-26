set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	surfviewer.cpp
	surfviewer.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(surfviewer
	${SOURCES}
)

target_include_directories(surfviewer PRIVATE 
	${WAPP_INCDIRS}
)

target_working_directory(surfviewer ${WG_ROOT} )

target_link_libraries(surfviewer PRIVATE
	${WAPPLIB}
)


