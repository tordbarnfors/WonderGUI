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
	${WAXSOFTGFX_INCDIRS}
	${WAXGFXSTREAM_INCDIRS}
	${WAXFREETYPEFONT_INCDIRS}
)


target_working_directory(streamgenerator ${WG_ROOT} )

target_link_libraries(streamgenerator PRIVATE
	waxsoftgfx
	waxfreetypefont
	wondergui
	waxgfxstream
	${SDL2_LIBS}
	${SDL2IMAGE_LIBS}
	${FREETYPE2_LIB}
)


