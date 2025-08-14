set(CMAKE_CXX_STANDARD 20)

source_group(Files REGULAR_EXPRESSION ".*")


set(SOURCES

	main.cpp
    sdlwindow.h
	sdlwindow.cpp
	tinyfiledialogs.h
	tinyfiledialogs.c
	../common/wonderapp.h
	../common/window.h
	../common/window.cpp
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${WONDERGUI_INCDIRS}
	${WGFREETYPEFONT_INCDIRS}
	${PROJECT_SOURCE_DIR}/wonderapp/common
	${PROJECT_SOURCE_DIR}
)

### Software rendering version

add_library(wappsdl2sw STATIC
    ${SOURCES}
    sdlwindow_sw.h
    sdlwindow_sw.cpp
)

target_include_directories(wappsdl2sw PRIVATE
    ${INCDIRS}
    ${WGSOFTGFX_INCDIRS}
    ${WGDEBUGGER_INCDIRS}
    ${SDL2_INCDIR}
    ${SDL2IMAGE_INCDIR}
)

set_target_properties(wappsdl2sw PROPERTIES FOLDER "WonderApp" )

target_link_directories(wappsdl2sw PUBLIC
    ${SDL2_LIBDIR}
    ${SDL2IMAGE_LIBDIR}
    ${FREETYPE2_LIBDIR}
)

target_link_libraries(wappsdl2sw PRIVATE
    wgsoftgfx
    wgfreetypefont
    wondergui
    wgdebugger
    wg_theme_simplistic
    ${FREETYPE2_LIB}
    ${SDL2_LIBS}
    ${SDL2IMAGE_LIBS}
)

### OpenGL-version

add_library(wappsdl2gl STATIC
    ${SOURCES}
    sdlwindow_gl.h
    sdlwindow_gl.cpp
)

#target_compile_definitions(wappsdl2gl PRIVATE USE_OPENGL)

target_include_directories(wappsdl2gl PRIVATE
    ${INCDIRS}
    ${WGOPENGLGFX_INCDIRS}
    ${WGDEBUGGER_INCDIRS}
    ${SDL2_INCDIR}
    ${SDL2IMAGE_INCDIR}
)

set_target_properties(wappsdl2gl PROPERTIES FOLDER "WonderApp" )

target_link_directories(wappsdl2gl PUBLIC
    ${SDL2_LIBDIR}
    ${SDL2IMAGE_LIBDIR}
    ${FREETYPE2_LIBDIR}
    ${GLEW_LIBDIR}
)

target_link_libraries(wappsdl2gl PRIVATE
    wgopenglgfx
    wgfreetypefont
    wondergui
    wgdebugger
    wg_theme_simplistic
    ${FREETYPE2_LIB}
    ${SDL2_LIBS}
    ${SDL2IMAGE_LIBS}
    ${OPENGL_LIBS}
)


### Metal-version

if(APPLE)

    add_library(wappsdl2metal STATIC
        ${SOURCES}
        sdlwindow_metal.h
        sdlwindow_metal.mm
    )

    #target_compile_definitions(wappsdl2metal PRIVATE USE_OPENGL)

    target_include_directories(wappsdl2metal PRIVATE
        ${INCDIRS}
        ${WGMETALGFX_INCDIRS}
        ${WGDEBUGGER_INCDIRS}
        ${SDL2_INCDIR}
        ${SDL2IMAGE_INCDIR}
    )

    set_target_properties(wappsdl2metal PROPERTIES FOLDER "WonderApp" )

    target_link_directories(wappsdl2metal PUBLIC
        ${SDL2_LIBDIR}
        ${SDL2IMAGE_LIBDIR}
        ${FREETYPE2_LIBDIR}
     )


    target_link_libraries(wappsdl2metal PRIVATE
        wgmetalgfx
        wgfreetypefont
        wondergui
        wgdebugger
        wg_theme_simplistic
        ${FREETYPE2_LIB}
        ${SDL2_LIBS}
        ${SDL2IMAGE_LIBS}
        ${APPKIT_LIB}
        ${METAL_LIB}
    )
endif()


    list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
    set(WAPP_SOURCES ${SOURCES} PARENT_SCOPE)
    set(WAPP_INCDIRS ${INCDIRS} PARENT_SCOPE)

