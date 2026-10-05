
set(SOURCES

	wg_glbackend.cpp
	wg_glbackend.h
	wg_glbackendshaders.cpp
	wg_glgfxdevicefactory.cpp
	wg_glgfxdevicefactory.h
	wg_glsurface.cpp
	wg_glsurface.h
	wg_glsurfacefactory.cpp
	wg_glsurfacefactory.h
	wg_gledgemap.cpp
	wg_gledgemap.h
	wg_gledgemapfactory.cpp
	wg_gledgemapfactory.h
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
)

#------------------------------------------------------------------------------
# OpenGL and GLEW
#
# Only looked for here, so nothing needs them unless this extension is built.
# GLEW loads the OpenGL entry points on Windows and Linux. macOS has them in its
# OpenGL framework, see wg_glbackend.h.
#------------------------------------------------------------------------------

find_package(OpenGL REQUIRED)

if(NOT APPLE)
	find_package(GLEW QUIET)

	if(NOT GLEW_FOUND AND WONDERGUI_FETCH_DEPS)
		include(FetchContent)
		set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
		set(ONLY_LIBS ON CACHE BOOL "" FORCE)
		FetchContent_Declare(
			glew
			GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
			GIT_TAG        glew-cmake-2.3.1
		)
		FetchContent_MakeAvailable(glew)
		add_library(GLEW::GLEW ALIAS libglew_static)
	endif()

	if(NOT TARGET GLEW::GLEW)
		message(FATAL_ERROR "GLEW not found. Install it, or enable WONDERGUI_FETCH_DEPS to download it.")
	endif()
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wgopenglgfx STATIC ${SOURCES} )

target_include_directories(wgopenglgfx PRIVATE 
	${WONDERGFX_INCDIRS}
	${INCDIRS}
)

# Public, so whatever links wgopenglgfx gets the GL headers and libraries too.

target_link_libraries(wgopenglgfx PUBLIC wondergfx OpenGL::GL)

if(NOT APPLE)
	target_link_libraries(wgopenglgfx PUBLIC GLEW::GLEW)
endif()

set_target_properties( wgopenglgfx PROPERTIES FOLDER "Extensions" )

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGOPENGLGFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(WGOPENGLGFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
