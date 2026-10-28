
set(SOURCES

	c-api/wax_c_softgfx.cpp
	c-api/wax_c_softgfx.h
	c-api/wax_c_lineargfx.cpp
	c-api/wax_c_lineargfx.h
	c-api/wax_c_softkernels_default.h
	c-api/wax_c_softkernels_tinylinear.h
	c-api/wax_c_softkernels_rgb555be_base.h
	c-api/wax_c_softkernels_rgb555be_extras.h
	c-api/wax_c_softkernels_bgr565srgb_base.h
	c-api/wax_c_softkernels_bgr565srgb_extras.h
	c-api/wax_c_softkernels_bgr565linear_base.h
	c-api/wax_c_softkernels_bgr565linear_extras.h

	wax_linearbackend.cpp
	wax_linearbackend.h
	wax_referencebackend.cpp
	wax_referencebackend.h
	wax_softtint.cpp
	wax_softtint.h
	wax_softbackend.cpp
	wax_softbackend.h
	wax_softbackend_warp.cpp
	wax_softkernels_default.cpp
	wax_softkernels_default.h


	wax_softkernels_tinylinear.cpp
	wax_softkernels_tinylinear.h

	wax_softkernels_bgr565srgb_base.cpp
	wax_softkernels_bgr565srgb_base.h
	wax_softkernels_bgr565srgb_extras.cpp
	wax_softkernels_bgr565srgb_extras.h

	wax_softkernels_bgr565linear_base.cpp
	wax_softkernels_bgr565linear_base.h
	wax_softkernels_bgr565linear_extras.cpp
	wax_softkernels_bgr565linear_extras.h

	wax_softkernels_rgb555be_base.cpp
	wax_softkernels_rgb555be_base.h
	wax_softkernels_rgb555be_extras.cpp
	wax_softkernels_rgb555be_extras.h

	wax_softkernelstanza.impl.h
	wax_softdevicefactory.cpp
	wax_softdevicefactory.h	
	wax_softsurface.cpp
	wax_softsurface.h
	wax_softsurfacefactory.cpp
	wax_softsurfacefactory.h
	wax_softedgemap.cpp
	wax_softedgemap.h
	wax_softedgemapfactory.cpp
	wax_softedgemapfactory.h
	wax_softwarpmap.cpp
	wax_softwarpmap.h
	wax_softwarpmapfactory.cpp
	wax_softwarpmapfactory.h
	wax_softshapes.cpp
	wax_softshapes.h
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/c-api
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( waxsoftgfx STATIC ${SOURCES} )

# Declared so the linker gets the libraries in the right order. GNU ld needs that,
# MSVC and Apple's linker don't care.
target_link_libraries( waxsoftgfx PUBLIC wax )
generate_export_header(waxsoftgfx BASE_NAME wax)

target_include_directories(waxsoftgfx PRIVATE 
	${WAX_INCDIRS}
	${INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)



list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WAXSOFTGFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAXSOFTGFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
