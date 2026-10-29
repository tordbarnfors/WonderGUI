set(CMAKE_CXX_STANDARD 20)

set(SOURCES

    ../../wonderapp.h
    ../../wappapi.h
    ../../wappwindowapi.h
    ../../wappwindow.h
    ../../wappwindow.cpp
    ../../wappimageloader.h
    ../../wappimageloader.cpp

    ../cocoagfxbackend.h
    ../main.mm
    ../cocoaapi.h
    ../cocoaapi.mm
    ../cocoawindow.h
    ../cocoawindow.mm

    ../../../../thirdparty/tinyfiledialogs/tinyfiledialogs.h
    ../../../../thirdparty/tinyfiledialogs/tinyfiledialogs.c

    cocoawindow_sw.mm
)

# Show the files the way they are laid out in wonderapp/source, with the third
# party ones in a group of their own.

set(WAPP_FILES ${SOURCES})
list(FILTER WAPP_FILES EXCLUDE REGEX "/thirdparty/")
list(TRANSFORM WAPP_FILES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )

source_group(TREE ${PROJECT_SOURCE_DIR}/wonderapp/source FILES ${WAPP_FILES})
source_group(thirdparty REGULAR_EXPRESSION "/thirdparty/")

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${WONDERGUI_INCDIRS}
	${WAXFREETYPEFONT_INCDIRS}
	${WGDEBUGGER_INCDIRS}
	${PROJECT_SOURCE_DIR}/wonderapp/source
	${PROJECT_SOURCE_DIR}/wonderapp/source/cocoa
	${STB_INCDIR}
	${PROJECT_SOURCE_DIR}/thirdparty/tinyfiledialogs
	${PROJECT_SOURCE_DIR}
)

### Software rendering version

add_library(wappcocoasw STATIC
    ${SOURCES}
)

target_include_directories(wappcocoasw PRIVATE
    ${INCDIRS}
    ${WAXSOFTGFX_INCDIRS}
)

target_link_libraries(wappcocoasw PRIVATE
    waxsoftgfx
    waxfreetypefont
    wondergui
    wgdebugger
)

# Static library, so the frameworks need to reach the app.

target_link_libraries(wappcocoasw PUBLIC
    "-framework Cocoa"
    "-framework QuartzCore"
    "-framework UniformTypeIdentifiers"
)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WAPP_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAPP_INCDIRS ${INCDIRS} PARENT_SCOPE)
