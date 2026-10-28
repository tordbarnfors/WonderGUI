set(CMAKE_CXX_STANDARD 20)

source_group(Files REGULAR_EXPRESSION ".*")


set(SOURCES

    ../../wonderapp.h
    ../../wappapi.h
    ../../wappwindowapi.h
    ../../wappwindow.h
    ../../wappwindow.cpp
    ../../wappimageloader.h
    ../../wappimageloader.cpp

    ../win32gfxbackend.h
    ../main.cpp
    ../win32api.h
    ../win32api.cpp

	win32window.h
    win32window.cpp
    win32gfxbackend.cpp

    dx12_wrapper.h
    dx12_wrapper.cpp
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${WONDERGUI_INCDIRS}
	${WAXFREETYPEFONT_INCDIRS}
	${WGDEBUGGER_INCDIRS}
	${PROJECT_SOURCE_DIR}/wonderapp/source
	${PROJECT_SOURCE_DIR}/wonderapp/source/win32
	${STB_INCDIR}
	${PROJECT_SOURCE_DIR}
)

### DX12 rendering version

add_library(wappwin32dx12 STATIC
    ${SOURCES}

)

target_include_directories(wappwin32dx12 PRIVATE
    ${INCDIRS}
    ${DX12GFX_INCDIRS}
)

set_target_properties(wappwin32dx12 PROPERTIES FOLDER "WonderApp" )

target_link_directories(wappwin32dx12 PUBLIC
    ${FREETYPE2_LIBDIR}
)

target_link_libraries(wappwin32dx12 PRIVATE
    waxdx12gfx
    waxfreetypefont
    wondergui
    wgdebugger
    ${FREETYPE2_LIB}
)




    list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
    set(WAPP_SOURCES ${SOURCES} PARENT_SCOPE)
    set(WAPP_INCDIRS ${INCDIRS} PARENT_SCOPE)

