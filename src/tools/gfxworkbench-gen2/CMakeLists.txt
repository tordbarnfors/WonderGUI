set(CMAKE_CXX_STANDARD 20)


set( SOURCES

	main.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(gfxworkbench-gen2
	${SOURCES}
)

target_include_directories(gfxworkbench-gen2 PRIVATE 
	${SDL2_INCDIR}
	${SDL2IMAGE_INCDIR}
	${WONDERGUI_INCDIRS}
	${WGSOFTGFX_INCDIRS}
	${GFXSTREAM_INCDIRS}

)


target_link_directories(gfxworkbench-gen2 PRIVATE 
	${SDL2_LIBDIR}
	${SDL2IMAGE_LIBDIR}
	${GLEW_LIBDIR}
)


target_working_directory(gfxworkbench-gen2 ${WG_ROOT})

target_link_libraries(gfxworkbench-gen2 PRIVATE
	wgsoftgfx
	wgopenglgfx
	wggfxstream
	wondergui 
	wgfreetypefont
	${SDL2_LIBS}
	${SDL2IMAGE_LIBS}
	${OPENGL_LIBS}
)
