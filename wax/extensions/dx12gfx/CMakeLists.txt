
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

# The shaders are shared with VulkanBackend, see ../hlsl/README.md.

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../hlsl)

set(SHADERS

	../hlsl/constants.hlsl
	../hlsl/output.hlsl
	../hlsl/ps_blit.hlsl
	../hlsl/ps_blit_palette.hlsl
	../hlsl/ps_blur.hlsl
	../hlsl/ps_blur_palette.hlsl
	../hlsl/ps_fill.hlsl
	../hlsl/ps_fill_aa.hlsl
	../hlsl/ps_line.hlsl
	../hlsl/ps_mipmap.hlsl
	../hlsl/ps_segments.hlsl
	../hlsl/ps_shape.hlsl
	../hlsl/ps_warp_blit.hlsl
	../hlsl/ps_warp_blit_palette.hlsl
	../hlsl/ps_warp_edgemap.hlsl
	../hlsl/tint.hlsl
	../hlsl/vs_blit.hlsl
	../hlsl/vs_fill.hlsl
	../hlsl/vs_fill_aa.hlsl
	../hlsl/vs_line.hlsl
	../hlsl/vs_mipmap.hlsl
	../hlsl/vs_segments.hlsl
	../hlsl/vs_shape.hlsl
	../hlsl/vs_warp.hlsl
	../hlsl/warp.hlsl
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
