
set(SOURCES

	wax_glbackend.cpp
	wax_glbackend.h
	wax_glbackendshaders.cpp
	wax_glgfxdevicefactory.cpp
	wax_glgfxdevicefactory.h
	wax_glsurface.cpp
	wax_glsurface.h
	wax_glsurfacefactory.cpp
	wax_glsurfacefactory.h
	wax_gledgemap.cpp
	wax_gledgemap.h
	wax_gledgemapfactory.cpp
	wax_gledgemapfactory.h
	wax_glwarpmap.cpp
	wax_glwarpmap.h
	wax_glwarpmapfactory.cpp
	wax_glwarpmapfactory.h
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
)

#------------------------------------------------------------------------------
# OpenGL and GLEW
#
# Only looked for here, so nothing needs them unless this extension is built.
# GLEW loads the OpenGL entry points on Windows and Linux. macOS has them in its
# OpenGL framework, see wax_glbackend.h.
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
		block()
			set(CMAKE_FOLDER thirdparty)
			FetchContent_MakeAvailable(glew)
		endblock()
		add_library(GLEW::GLEW ALIAS libglew_static)
	endif()

	if(NOT TARGET GLEW::GLEW)
		message(FATAL_ERROR "GLEW not found. Install it, or enable WONDERGUI_FETCH_DEPS to download it.")
	endif()
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( waxopenglgfx STATIC ${SOURCES} )

target_include_directories(waxopenglgfx PRIVATE 
	${WAX_INCDIRS}
	${INCDIRS}
)

# Public, so whatever links waxopenglgfx gets the GL headers and libraries too.

target_link_libraries(waxopenglgfx PUBLIC wax OpenGL::GL)

if(NOT APPLE)
	target_link_libraries(waxopenglgfx PUBLIC GLEW::GLEW)
endif()

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WAXOPENGLGFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAXOPENGLGFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
