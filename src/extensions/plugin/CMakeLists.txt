
set( PLUGIN_SOURCES
	wonderplugin.h

	wg_c_plugincapsule.h
	wg_c_pluginroot.cpp
	wg_c_pluginroot.h

	wg_pluginbase.cpp
	wg_pluginbase.h
	wg_pluginroot.cpp
	wg_pluginroot.h

	wg_plugincalls.cpp
	wg_plugincalls.h
	wg_pluginbitmapcache.cpp
	wg_pluginbitmapcache.h
	wg_pluginbitmapfont.cpp
	wg_pluginbitmapfont.h
	wg_plugincanvaslayers.cpp
	wg_plugincanvaslayers.h
	wg_pluginfont.cpp
	wg_pluginfont.h
	wg_pluginfreetypefont.cpp
	wg_pluginfreetypefont.h
	wg_plugingfxdevice.cpp
	wg_plugingfxdevice.h
	wg_pluginhostbridge.cpp
	wg_pluginhostbridge.h
	wg_plugininterface.h
	wg_pluginsurface.cpp
	wg_pluginsurface.h
	wg_pluginsurfacefactory.cpp
	wg_pluginsurfacefactory.h
	wg_pluginedgemap.cpp
	wg_pluginedgemap.h
	wg_pluginedgemapfactory.cpp
	wg_pluginedgemapfactory.h

	wonderplugin.h
)

set( LEGACYPLUGIN_SOURCES
	wglegacyplugin.h

	wg_c_plugincapsule.h

	wg2_c_pluginroot.cpp
	wg2_c_pluginroot.h

	wg2_pluginbase.cpp
	wg2_pluginbase.h
	wg2_pluginroot.cpp
	wg2_pluginroot.h

	wg_plugincalls.cpp
	wg_plugincalls.h
	wg_pluginbitmapcache.cpp
	wg_pluginbitmapcache.h
	wg_pluginbitmapfont.cpp
	wg_pluginbitmapfont.h
	wg_plugincanvaslayers.cpp
	wg_plugincanvaslayers.h
	wg_pluginfont.cpp
	wg_pluginfont.h
	wg_pluginfreetypefont.cpp
	wg_pluginfreetypefont.h
	wg_plugingfxdevice.cpp
	wg_plugingfxdevice.h
	wg_pluginhostbridge.cpp
	wg_pluginhostbridge.h
	wg_plugininterface.h
	wg_pluginsurface.cpp
	wg_pluginsurface.h
	wg_pluginsurfacefactory.cpp
	wg_pluginsurfacefactory.h	
	wg_pluginedgemap.cpp
	wg_pluginedgemap.h
	wg_pluginedgemapfactory.cpp
	wg_pluginedgemapfactory.h
)

set( HOST_SOURCES

	wg_plugininterface.cpp
	wg_plugininterface.h

	wg_plugincapsule.cpp
	wg_plugincapsule.h

	wg_c_plugincapsule.cpp
	wg_c_plugincapsule.h
	
	wg_c_pluginroot.h
)

set(INCDIRS
	 ${CMAKE_CURRENT_LIST_DIR}
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PLUGIN_SOURCES})

add_library( wgplugin STATIC ${PLUGIN_SOURCES} )
generate_export_header(wgplugin BASE_NAME wg)

target_include_directories(wgplugin PRIVATE
	${WONDERGUI_INCDIRS}
	${INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LEGACYPLUGIN_SOURCES})

add_library( wglegacyplugin STATIC ${LEGACYPLUGIN_SOURCES} )
generate_export_header(wglegacyplugin BASE_NAME wg)

target_include_directories(wglegacyplugin PRIVATE
	${WGLEGACYGUI_INCDIRS}
	${INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HOST_SOURCES})

add_library( wgpluginhost STATIC ${HOST_SOURCES} )
generate_export_header(wgpluginhost BASE_NAME wg)

target_include_directories(wgpluginhost PRIVATE
	${WONDERGUI_INCDIRS}
	${INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

set_target_properties( wgpluginhost wgplugin wglegacyplugin PROPERTIES FOLDER "Extensions" )

 
list(TRANSFORM PLUGIN_SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGPLUGIN_SOURCES ${PLUGIN_SOURCES} PARENT_SCOPE)
set(WGPLUGIN_INCDIRS ${INCDIRS} PARENT_SCOPE)

list(TRANSFORM LEGACYPLUGIN_SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGLEGACYPLUGIN_SOURCES ${LEGACYPLUGIN_SOURCES} PARENT_SCOPE)
set(WGLEGACYPLUGIN_INCDIRS ${INCDIRS} PARENT_SCOPE)

