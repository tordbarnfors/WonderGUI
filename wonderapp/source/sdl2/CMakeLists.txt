set(CMAKE_CXX_STANDARD 20)

#------------------------------------------------------------------------------
# SDL2
#
# Only looked for here, so nothing needs SDL2 unless an SDL2 WonderApp
# framework is selected. Everything else uses SDL3.
#------------------------------------------------------------------------------

find_package(SDL2 CONFIG QUIET)

if(NOT SDL2_FOUND AND WONDERGUI_FETCH_DEPS)
    include(FetchContent)

    # With MSVC, SDL2 builds without the C runtime by default and links its DLL
    # with /NODEFAULTLIB. It keeps runtime checks (/RTC1), which need the C
    # runtime, out by removing them from CMAKE_C_FLAGS_DEBUG. CMake 4 (policy
    # CMP0184, NEW for everything since we require 4.4) adds them through the
    # MSVC_RUNTIME_CHECKS target property instead, so they stay and the link
    # fails on _RTC_CheckStackVars. Building SDL2 with the C runtime avoids it.

    set(SDL_LIBC ON CACHE BOOL "Use the system C library" FORCE)

    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-2.30.0
    )
    block()
        set(CMAKE_FOLDER thirdparty)
        FetchContent_MakeAvailable(SDL2)
    endblock()
    # SDL2 sets up SDL2::SDL2 / SDL2::SDL2main targets itself when built this way
endif()

# SDL.h turns main() into SDL_main() on Windows, which SDL2main then calls.

if(NOT SDL2_LIBS AND TARGET SDL2::SDL2)
    set(SDL2_LIBS SDL2::SDL2)
    if(WIN32 AND TARGET SDL2::SDL2main)
        set(SDL2_LIBS SDL2::SDL2main SDL2::SDL2)
    endif()
endif()

if(NOT SDL2_LIBS)
    message(FATAL_ERROR "SDL2 not found. Install it, or enable WONDERGUI_FETCH_DEPS to download it.")
endif()

set(SOURCES

	main.cpp
    sdlwindow.h
	sdlwindow.cpp
	${PROJECT_SOURCE_DIR}/thirdparty/tinyfiledialogs/tinyfiledialogs.h
	${PROJECT_SOURCE_DIR}/thirdparty/tinyfiledialogs/tinyfiledialogs.c
	../wappapi.h
	../wappwindow.h
	../wappwindow.cpp
	../wappimageloader.h
	../wappimageloader.cpp
    ../wappwindowapi.h
    ../wonderapp.h
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${WONDERGUI_INCDIRS}
	${WAXFREETYPEFONT_INCDIRS}
	${WGDEBUGGER_INCDIRS}
	${PROJECT_SOURCE_DIR}/wonderapp/source
	${STB_INCDIR}
	${PROJECT_SOURCE_DIR}/thirdparty/tinyfiledialogs
	${PROJECT_SOURCE_DIR}
)

# Only the selected framework gets a target, so nothing here needs OpenGL or
# Metal unless that is what was asked for.

### Software rendering version

if(WAPPLIB STREQUAL "wappsdl2sw")

add_library(wappsdl2sw STATIC
    ${SOURCES}
    backend_software.cpp
)

target_include_directories(wappsdl2sw PRIVATE
    ${INCDIRS}
    ${WAXSOFTGFX_INCDIRS}
    ${WGDEBUGGER_INCDIRS}
    ${SDL2_INCDIR}
)

target_link_directories(wappsdl2sw PUBLIC
    ${SDL2_LIBDIR}
    ${FREETYPE2_LIBDIR}
)

target_link_libraries(wappsdl2sw PRIVATE
    wgdebugger
    waxsoftgfx
    waxfreetypefont
    wondergui
    ${FREETYPE2_LIB}
    ${SDL2_LIBS}
)

endif()

### OpenGL-version

if(WAPPLIB STREQUAL "wappsdl2gl")

add_library(wappsdl2gl STATIC
    ${SOURCES}
    backend_opengl.cpp
)

#target_compile_definitions(wappsdl2gl PRIVATE USE_OPENGL)

target_include_directories(wappsdl2gl PRIVATE
    ${INCDIRS}
    ${WAXOPENGLGFX_INCDIRS}
    ${WGDEBUGGER_INCDIRS}
    ${SDL2_INCDIR}
)

target_link_directories(wappsdl2gl PUBLIC
    ${SDL2_LIBDIR}
    ${FREETYPE2_LIBDIR}
    ${GLEW_LIBDIR}
)

target_link_libraries(wappsdl2gl PRIVATE
    waxopenglgfx
    waxfreetypefont
    wondergui
    wgdebugger
    ${FREETYPE2_LIB}
    ${SDL2_LIBS}
    ${OPENGL_LIBS}
)

endif()

### Metal-version

if(WAPPLIB STREQUAL "wappsdl2metal")

    add_library(wappsdl2metal STATIC
        ${SOURCES}
        backend_metal.mm
    )

    #target_compile_definitions(wappsdl2metal PRIVATE USE_OPENGL)

    target_include_directories(wappsdl2metal PRIVATE
        ${INCDIRS}
        ${WAXMETALGFX_INCDIRS}
        ${WGDEBUGGER_INCDIRS}
        ${SDL2_INCDIR}
    )

    target_link_directories(wappsdl2metal PUBLIC
        ${SDL2_LIBDIR}
        ${FREETYPE2_LIBDIR}
     )


    target_link_libraries(wappsdl2metal PRIVATE
        waxmetalgfx
        waxfreetypefont
        wondergui
        wgdebugger
        ${FREETYPE2_LIB}
        ${SDL2_LIBS}
        ${APPKIT_LIB}
        ${METAL_LIB}
    )
endif()


    set(WAPP_SOURCES ${SOURCES} PARENT_SCOPE)
    set(WAPP_INCDIRS ${INCDIRS} PARENT_SCOPE)


# Show the files the way they are laid out in wonderapp/source, with the third
# party ones in a group of their own.

get_target_property(WAPP_FILES ${WAPPLIB} SOURCES)
set(THIRDPARTY_FILES ${WAPP_FILES})
list(FILTER WAPP_FILES EXCLUDE REGEX "/thirdparty/")
list(FILTER THIRDPARTY_FILES INCLUDE REGEX "/thirdparty/")

source_group(TREE ${PROJECT_SOURCE_DIR}/wonderapp/source FILES ${WAPP_FILES})
source_group(thirdparty FILES ${THIRDPARTY_FILES})
