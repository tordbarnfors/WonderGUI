set(CMAKE_CXX_STANDARD 20)

source_group(Files REGULAR_EXPRESSION ".*")


set(SOURCES

	main.cpp
    win32visitor.h
    win32visitor.cpp
    win32window.h
    win32window.cpp
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

add_library(wappwin32 STATIC
    ${SOURCES}
#    sdlwindow_sw.h
#    sdlwindow_sw.cpp
)

target_include_directories(wappwin32 PRIVATE
    ${INCDIRS}
    ${WGSOFTGFX_INCDIRS}
    ${WGDEBUGGER_INCDIRS}
    ${SDL2_INCDIR}
    ${SDL2IMAGE_INCDIR}
)

set_target_properties(wappwin32 PROPERTIES FOLDER "WonderApp" )

target_link_directories(wappwin32 PUBLIC
    ${SDL2_LIBDIR}
    ${SDL2IMAGE_LIBDIR}
    ${FREETYPE2_LIBDIR}
)

target_link_libraries(wappwin32 PRIVATE
    wgsoftgfx
    wgfreetypefont
    wondergui
    wgdebugger
    wg_theme_simplistic
    ${FREETYPE2_LIB}
    ${SDL2_LIBS}
    ${SDL2IMAGE_LIBS}
)




    list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
    set(WAPP_SOURCES ${SOURCES} PARENT_SCOPE)
    set(WAPP_INCDIRS ${INCDIRS} PARENT_SCOPE)

