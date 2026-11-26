set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	bitmapviewer.cpp
	bitmapviewer.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(bitmapviewer
	${SOURCES}
)

target_include_directories(bitmapviewer PRIVATE 
	${WAPP_INCDIRS}
)

target_working_directory(bitmapviewer ${WG_ROOT} )

target_link_libraries(bitmapviewer PRIVATE
	${WAPPLIB}
)


