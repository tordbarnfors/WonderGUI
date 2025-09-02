set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	streamplayer.cpp
	streamplayer.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(streamplayer ${SOURCES})
generate_export_header(wondergfx BASE_NAME wg)

target_include_directories(streamplayer PRIVATE 
	${WAPP_INCDIRS}
	${WGGFXSTREAM_INCDIRS}
	${WGSOFTGFX_INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

target_working_directory(streamplayer ${WG_ROOT} )

target_link_libraries(streamplayer PRIVATE
	${WAPPLIB}
	wgsoftgfx
	wggfxstream
)


