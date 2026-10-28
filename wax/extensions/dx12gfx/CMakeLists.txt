
set(SOURCES

	wax_dx12backend.cpp
	wax_dx12backend.h
	wax_dx12edgemap.cpp
	wax_dx12edgemap.h
	wax_dx12edgemapfactory.cpp
	wax_dx12edgemapfactory.h
	wax_dx12warpmap.cpp
	wax_dx12warpmap.h
	wax_dx12warpmapfactory.cpp
	wax_dx12warpmapfactory.h
	wax_dx12shaders.cpp
	wax_dx12shaders.cpp.template
	wax_dx12surface.cpp
	wax_dx12surface.h
	wax_dx12surfacefactory.cpp
	wax_dx12surfacefactory.h
)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

set(SHADERS

	shaders/vs_fill.hlsl
	shaders/ps_fill.hlsl
	shaders/vs_fill_aa.hlsl
	shaders/ps_fill_aa.hlsl
	shaders/vs_blit.hlsl
	shaders/ps_blit.hlsl
	shaders/ps_blur.hlsl
	shaders/ps_blit_palette.hlsl
	shaders/ps_blur_palette.hlsl
	shaders/vs_line.hlsl
	shaders/ps_line.hlsl
	shaders/vs_segments.hlsl
	shaders/ps_segments.hlsl
	shaders/vs_shape.hlsl
	shaders/ps_shape.hlsl
	shaders/vs_warp.hlsl
	shaders/warp.hlsl
	shaders/ps_warp_edgemap.hlsl
	shaders/ps_warp_blit.hlsl
	shaders/ps_warp_blit_palette.hlsl
)



set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
#source_group(TREE "Shaders" FILES ${SHADERS})

add_library( waxdx12gfx STATIC ${SOURCES} ${SHADERS} )

# Declared so the linker gets the libraries in the right order. GNU ld needs that,
# MSVC and Apple's linker don't care.
target_link_libraries( waxdx12gfx PUBLIC wax )

target_include_directories(waxdx12gfx PRIVATE 
	${WAX_INCDIRS}
	${INCDIRS}
)


set_source_files_properties( wax_dx12shaders.cpp.template ${SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE )

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(DX12GFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(DX12GFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
