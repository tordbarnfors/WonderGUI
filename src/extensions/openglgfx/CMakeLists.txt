
set(SOURCES

	wg_glbackend.cpp
	wg_glbackend.h
	wg_glbackendshaders.cpp
	wg_glgfxdevice.cpp
	wg_glgfxdevice.h
	wg_glshaders.cpp
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
	${GLEW_INCDIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wgopenglgfx STATIC ${SOURCES} )

target_include_directories(wgopenglgfx PRIVATE 
	${WONDERGFX_INCDIRS}
	${INCDIRS}
)

set_target_properties( wgopenglgfx PROPERTIES FOLDER "Extensions" )

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGOPENGLGFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(WGOPENGLGFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
