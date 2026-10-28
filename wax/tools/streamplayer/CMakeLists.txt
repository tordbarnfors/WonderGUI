set(CMAKE_CXX_STANDARD 20)

set( SOURCES
	streamplayer.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(streamplayer ${SOURCES})

target_include_directories(streamplayer PRIVATE
	${WAX_INCDIRS}
	${WAXGFXSTREAM_INCDIRS}
	${WAXSOFTGFX_INCDIRS}
)

target_working_directory(streamplayer ${WG_ROOT} )

target_link_libraries(streamplayer PRIVATE
	waxsoftgfx
	waxgfxstream
	${SDL3_LIBS}
)
