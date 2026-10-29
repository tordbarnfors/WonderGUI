
set(SOURCES

	wax_metalbackend.h
	wax_metalbackend.mm
	wax_metaldevicefactory.h
	wax_metaldevicefactory.mm
	wax_metalshaders.mm
	wax_metalshaders.mm.template
	wax_metalsurface.h
	wax_metalsurface.mm
	wax_metalsurfacefactory.h
	wax_metalsurfacefactory.mm
	wax_metaledgemap.h
	wax_metaledgemap.mm
	wax_metaledgemapfactory.h
	wax_metaledgemapfactory.mm
	wax_metalwarpmap.h
	wax_metalwarpmap.mm
	wax_metalwarpmapfactory.h
	wax_metalwarpmapfactory.mm
	wax_shadercontent.mm
)

set_source_files_properties(wax_metalshaders.mm.template wax_shadercontent.mm PROPERTIES HEADER_FILE_ONLY YES)


set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( waxmetalgfx STATIC ${SOURCES} )

# Declared so the linker gets the libraries in the right order. GNU ld needs that,
# MSVC and Apple's linker don't care.
target_link_libraries( waxmetalgfx PUBLIC wax )

# The Objective-C++ sources use Foundation (NSString, NSAutoreleasePool) as well
# as Metal and QuartzCore, so anything linking this library needs those too.
target_link_libraries( waxmetalgfx PUBLIC
	"-framework Foundation"
	"-framework Metal"
	"-framework QuartzCore"
)

target_include_directories(waxmetalgfx PRIVATE 
	${WAX_INCDIRS}
	${INCDIRS}
)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WAXMETALGFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAXMETALGFX_INCDIRS ${INCDIRS} PARENT_SCOPE)
