
set(SOURCES

	wg_metalbackend.h
	wg_metalbackend.mm
	wg_metalgfxdevicefactory.h
	wg_metalgfxdevicefactory.mm
	wg_metalshaders.mm
	wg_metalshaders.mm.template
	wg_metalsurface.h
	wg_metalsurface.mm
	wg_metalsurfacefactory.h
	wg_metalsurfacefactory.mm
	wg_metaledgemap.h
	wg_metaledgemap.mm
	wg_metaledgemapfactory.h
	wg_metaledgemapfactory.mm
	wg_shadercontent.mm
)

set_source_files_properties(wg_metalshaders.mm.template wg_shadercontent.mm PROPERTIES HEADER_FILE_ONLY YES)


set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wgmetalgfx STATIC ${SOURCES} )

target_include_directories(wgmetalgfx PRIVATE 
	${WONDERGFX_INCDIRS}
	${INCDIRS}
)

set_target_properties( wgmetalgfx PROPERTIES FOLDER "Extensions" )

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGMETALGFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(WGMETALGFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
