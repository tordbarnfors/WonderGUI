
set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	main.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(workbench
	${SOURCES}
)

target_working_directory(workbench ${WG_ROOT} )

target_include_directories(workbench PRIVATE 
	${WONDERGUI_INCDIRS}
	${WAXSOFTGFX_INCDIRS}
	${WAXOPENGLGFX_INCDIRS}
	${WAXFREETYPEFONT_INCDIRS}
	${WGDEBUGGER_INCDIRS}
	${STB_INCDIR}
	${PROJECT_SOURCE_DIR}


)

target_link_directories(workbench PRIVATE 
	${GLEW_LIBDIR}
	${FREETYPE2_LIBDIR}
)


target_link_libraries(workbench PRIVATE
	wgdebugger
	waxsoftgfx
	waxopenglgfx
	waxfreetypefont
	wondergui 
	${SDL3_LIBS}
	${OPENGL_LIBS}
	${FREETYPE2_LIB}
)

