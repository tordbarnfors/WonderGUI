set(CMAKE_CXX_STANDARD 20)

source_group(Files REGULAR_EXPRESSION ".*")


set(SOURCES

    ../common/wonderapp.h
    ../common/wappapi.h
    ../common/wappwindowapi.h
    ../common/wappwindow.h
    ../common/wappwindow.cpp

	main.cpp
    win32api.h
    win32api.cpp
    win32window.h
    win32window.cpp
    stb_image.h

    dx12_wrapper.h
    dx12_wrapper.cpp
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${WONDERGUI_INCDIRS}
	${WGFREETYPEFONT_INCDIRS}
	${WGDEBUGGER_INCDIRS}
	${PROJECT_SOURCE_DIR}/wonderapp/common
	${PROJECT_SOURCE_DIR}
)

### DX12 rendering version

add_library(wappwin32dx12 STATIC
    ${SOURCES}

)

target_include_directories(wappwin32dx12 PRIVATE
    ${INCDIRS}
    ${DX12GFX_INCDIRS}
    ${SDL2_INCDIR}
    ${SDL2IMAGE_INCDIR}
)

set_target_properties(wappwin32dx12 PROPERTIES FOLDER "WonderApp" )

target_link_directories(wappwin32dx12 PUBLIC
    ${SDL2_LIBDIR}
    ${SDL2IMAGE_LIBDIR}
    ${FREETYPE2_LIBDIR}
)

target_link_libraries(wappwin32dx12 PRIVATE
    wgdx12gfx
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

