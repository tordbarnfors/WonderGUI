set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	main.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(streamgenerator
	${SOURCES}
)


target_link_directories(streamgenerator PRIVATE 
	${FREETYPE2_LIBDIR}
	${SDL2_LIBDIR}
	${SDL2IMAGE_LIBDIR}
)

target_include_directories(streamgenerator PRIVATE 
	${WONDERGUI_INCDIRS}
	${WGSOFTGFX_INCDIRS}
	${WGGFXSTREAM_INCDIRS}
	${WGFREETYPEFONT_INCDIRS}
	${WG_ROOT}/SDL2/include/
	${WG_ROOT}/SDL2_image/include/
)


target_working_directory(streamgenerator ${WG_ROOT} )

target_link_libraries(streamgenerator PRIVATE
	wgsoftgfx
	wgfreetypefont
	wondergui
	wggfxstream
	${SDL2_LIBS}
	${SDL2IMAGE_LIBS}
	${FREETYPE2_LIB}
)


