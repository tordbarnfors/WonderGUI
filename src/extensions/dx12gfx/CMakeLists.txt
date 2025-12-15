
set(SOURCES

	wg_dx12backend.cpp
	wg_dx12backend.h
	wg_dx12edgemap.cpp
	wg_dx12edgemap.h
	wg_dx12edgemapfactory.cpp
	wg_dx12edgemapfactory.h
	wg_dx12shaders.cpp
	wg_dx12shaders.cpp.template
	wg_dx12surface.cpp
	wg_dx12surface.h
	wg_dx12surfacefactory.cpp
	wg_dx12surfacefactory.h
)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

set(SHADERS

	shaders/vs_fill.hlsl
	shaders/ps_fill.hlsl
)



set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
#source_group(TREE "Shaders" FILES ${SHADERS})

add_library( wgdx12gfx STATIC ${SOURCES} ${SHADERS} )

target_include_directories(wgdx12gfx PRIVATE 
	${WONDERGFX_INCDIRS}
	${INCDIRS}
)


set_target_properties( wgdx12gfx PROPERTIES FOLDER "Extensions" )

set_source_files_properties( wg_dx12shaders.cpp.template ${SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE )

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(DX12GFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(DX12GFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
