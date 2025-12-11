
set(SOURCES

	wg_dx12backend.cpp
	wg_dx12backend.h
	wg_dx12surface.cpp
	wg_dx12surface.h
	wg_dx12surfacefactory.cpp
	wg_dx12surfacefactory.h
	wg_dx12edgemap.cpp
	wg_dx12edgemap.h
	wg_dx12edgemapfactory.cpp
	wg_dx12edgemapfactory.h
	wg_dx12shaders.cpp
)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

set(SHADERS

	${SHADER_DIR}/vs_fill.hlsl
	${SHADER_DIR}/ps_fill.hlsl
)

set(GENERATED_SHADERS "wg_dx12shaders.cpp" )

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wgdx12gfx STATIC ${SOURCES} )

target_include_directories(wgdx12gfx PRIVATE 
	${WONDERGFX_INCDIRS}
	${INCDIRS}
)

add_custom_command(
    OUTPUT ${GENERATED_SHADERS}
    COMMAND ${CMAKE_COMMAND}
        -DSHADER_FILES="${SHADERS}"
        -DOUTPUT_FILE=${GENERATED_SHADERS}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_shaders.cmake
    DEPENDS ${SHADERS}
    COMMENT "Generating embedded shaders"
    VERBATIM
)

set_target_properties( wgdx12gfx PROPERTIES FOLDER "Extensions" )

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(DX12GFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(DX12GFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
