cmake_minimum_required(VERSION 3.16)

include(GenerateExportHeader)

#set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development")
#set(CMAKE_XCODE_ATTRIBUTE_CODE_DEVELOPMENT_TEAM 9B39M5G274)

#set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)

#set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)

project(WonderGUI VERSION 3.0 DESCRIPTION "WonderGUI User Interface Library")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(WG_ROOT ${PROJECT_SOURCE_DIR}/..)


#------------------------------------------------------------------------------
# DIRECTORIES
#
# SDL2, SDL2_image, GLEW and FreeType libraries are needed by several compile targets,
# such as tools, workbenches, examples and some WonderApp frameworks.
#
# The base libraries (WonderGear, WonderGfx, WonderGlyphs and WonderGUI) do not need these.
#
#------------------------------------------------------------------------------

if(WIN32)
	set(SDL2_INCDIR ${WG_ROOT}/SDL2/include)
	set(SDL2_LIBDIR ${WG_ROOT}/SDL2/lib/x64/)
	set(SDL2_LIBS SDL2 SDL2main)

	set(SDL2IMAGE_INCDIR ${WG_ROOT}/SDL2_image/include)
	set(SDL2IMAGE_LIBDIR ${WG_ROOT}/SDL2_image/lib/x64/)
	set(SDL2IMAGE_LIBS SDL2_image)

	set(FREETYPE2_INCDIR ${WG_ROOT}/freetype/include)
	set(FREETYPE2_LIBDIR ${WG_ROOT}/freetype/lib/x64/)
    set(FREETYPE2_LIB freetype)

	set(GLEW_INCDIR ${WG_ROOT}/glew/include)
	set(GLEW_LIBDIR ${WG_ROOT}/glew/lib/Release/x64/)	
	set(OPENGL_LIBS glew32 opengl32)

elseif(APPLE)


    set(FREETYPE2_LIB freetype)
    # Use the stable 'opt' path and the freetype2 subfolder
    set(FREETYPE2_INCDIR "/opt/homebrew/opt/freetype/include/freetype2;/opt/homebrew/include" CACHE STRING "Freetype include dirs (list)")
	set(FREETYPE2_LIBDIR  /opt/homebrew/opt/freetype/lib            CACHE FILEPATH "Path to libfreetype")
    # If you previously set FREETYPE2_LIB to 'freetype', keep it:
    set(FREETYPE2_LIB freetype)

    # Make sure the linker actually searches the lib dir:
    # (CMake 3.13+)
    add_link_options(-L${FREETYPE2_LIBDIR})
    # or per-target:
    # target_link_directories(streamgendll PRIVATE ${FREETYPE2_LIBDIR})

    #--------------------
    # Use local freetype instead of what is supplied by homebrew.
    #--------------------
	# set(FREETYPE2_INCDIR ${WG_ROOT}/freetype/include)
	# set(FREETYPE2_LIBDIR ${WG_ROOT}/freetype/lib/)
    # set(FREETYPE2_LIB freetype bz2 z brotlidec brotlicommon )

    find_library(SDL2_LIBS SDL2)
    find_library(SDL2IMAGE_LIBS SDL2_image)
    find_library(OPENGL_LIBS OpenGL)
    find_library(APPKIT_LIB AppKit)
    find_library(METAL_LIB Metal)


else()
    set(FREETYPE2_LIB freetype)
	set(FREETYPE2_INCDIR /usr/include/freetype2 CACHE STRING "Freetype include dirs (list)")
	set(FREETYPE2_LIBDIR "" CACHE FILEPATH "Path to libfreetype.so or .a")

	set(SDL2_LIBS SDL2)
	set(SDL2IMAGE_LIBS SDL2_image)

	set(OPENGL_LIBS GLEW GL)

endif()


#------------------------------------------------------------------------------
# WONDERAPP FRAMEWORK SELECTION
#
# Select which WonderApp framework to use when building tools and other included
# applications.
#
#------------------------------------------------------------------------------

#set(WAPPLIB wappwin32sw)			# WonderApp framework for Win32, using software rendering.
set(WAPPLIB wappwin32dx12)			# WonderApp framework for Win32, using software rendering.

#set(WAPPLIB wappsdl2sw)			# WonderApp framework based on SDL2, using software rendering.
									# Works on Windows, Mac and Linux.
#set(WAPPLIB wappsdl2gl)			# WonderApp framework based on SDL2, using OpenGL rendering.
									# Works on Windows, Mac and Linux, although deprecated on both Windows and Mac.
#set(WAPPLIB wappsdl2metal)			# WonderApp framework based on SDL2, using Metal rendering.
									# Works on Mac.


#------------------------------------------------------------------------------
# FUNCTIONS
#
# Used by subdirectories CMakeLists.
#------------------------------------------------------------------------------


function( target_working_directory target workpath )

	if(WIN32)
		set_target_properties( ${target} PROPERTIES 
			VS_DEBUGGER_WORKING_DIRECTORY ${workpath} 
		)

	elseif(APPLE)
		set_target_properties( ${target} PROPERTIES
			XCODE_GENERATE_SCHEME TRUE
			XCODE_SCHEME_WORKING_DIRECTORY "${workpath}"
		)
	endif()

endfunction()


#------------------------------------------------------------------------------
# COMPILER OPTIONS
#------------------------------------------------------------------------------

if(MSVC)
	add_compile_options("/MP")
endif()

#------------------------------------------------------------------------------

add_subdirectory(wondergear)
add_subdirectory(wondergfx)
add_subdirectory(wondergui)

add_subdirectory(extensions/debugger)

if(APPLE)
	add_subdirectory(extensions/metalgfx)
endif()

if(WIN32)
	add_subdirectory(extensions/dx12gfx)
endif()

add_subdirectory(extensions/openglgfx)
add_subdirectory(extensions/gfxstream)
add_subdirectory(extensions/softgfx)

add_subdirectory(extensions/freetypefont)

add_subdirectory(extensions/legacygui)

add_subdirectory(extensions/plugin)

#add_subdirectory(wonderapp/wappsdl2)
#add_subdirectory(wonderapp/win32-sw)
add_subdirectory(wonderapp/win32-dx12)

add_subdirectory(tools)
add_subdirectory(examples)
add_subdirectory(themes)


#install(TARGETS example01 DESTINATION ${WG_ROOT}/distribution/examples)

install(TARGETS streamanalyzer DESTINATION ${WG_ROOT}/distribution/tools)
