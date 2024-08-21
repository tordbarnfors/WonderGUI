
set( SOURCES

	wg_bitmapcache.cpp
	wg_bitmapcache.h
	wg_bitmapfont.cpp
	wg_bitmapfont.h
	wg_blurbrush.cpp
	wg_blurbrush.h
	wg_canvaslayers.cpp
	wg_canvaslayers.h
	wg_color.cpp
	wg_color.h
	wg_dummyfont.cpp
	wg_dummyfont.h
	wg_edgemap.cpp
	wg_edgemap.h
	wg_edgemapfactory.cpp
	wg_edgemapfactory.h
	wg_edgemaptools.cpp
	wg_edgemaptools.h
	wg_font.cpp
	wg_font.h
	wg_geo.cpp
	wg_geo.h
	wg_gfxbase.cpp
	wg_gfxbase.h
	wg_gfxdevice.cpp
	wg_gfxdevice.h
	wg_gfxdevice_gen2.cpp
	wg_gfxdevice_gen2.h
	wg_gfxdevice_gen1.cpp
	wg_gfxdevice_gen1.h
	wg_gfxbackend.cpp
	wg_gfxbackend.h
	wg_gfxenumextras.cpp
	wg_gfxenumextras.h
	wg_gfxtypes.h
	wg_gfxutil.cpp
	wg_gfxutil.h
	wg_gradient.cpp
	wg_gradient.h
	wg_gradyent.cpp
	wg_gradyent.h
	wg_gridwalker.cpp
	wg_gridwalker.h
	wg_backendlogger.cpp
	wg_backendlogger.h
	wg_nullgfxdevice.cpp
	wg_nullgfxdevice.h
	wg_patches.impl.h
	wg_patches.cpp
	wg_patches.h
	wg_pixeltools.cpp
	wg_pixeltools.h
	wg_printer.cpp
	wg_printer.h
	wg_surface.cpp
	wg_surface.h
	wg_surfacefactory.cpp
	wg_surfacefactory.h
	wg_surfacefileheader.h
	wg_surfacefileinspector.cpp
	wg_surfacefileinspector.h
	wg_surfacereader.cpp
	wg_surfacereader.h
	wg_surfacewriter.cpp
	wg_surfacewriter.h
	wg_sysfont.cpp
	wg_sysfont.h
	wg_tintmap.cpp
	wg_tintmap.h
	wg_transitions.cpp
	wg_transitions.h
	wg_waveform.cpp
	wg_waveform.h
	wondergfx.h

	c-api/wg_c_bitmapcache.cpp
	c-api/wg_c_bitmapcache.h
	c-api/wg_c_bitmapfont.cpp
	c-api/wg_c_bitmapfont.h	
	c-api/wg_c_canvaslayers.cpp
	c-api/wg_c_canvaslayers.h
	c-api/wg_c_color.cpp
	c-api/wg_c_color.h
	c-api/wg_c_font.cpp
	c-api/wg_c_font.h	
	c-api/wg_c_geo.cpp
	c-api/wg_c_geo.h
	c-api/wg_c_gfxbase.cpp
	c-api/wg_c_gfxbase.h
	c-api/wg_c_gfxdevice.cpp
	c-api/wg_c_gfxdevice.h
	c-api/wg_c_gfxtypes.h
	c-api/wg_c_gfxutil.cpp
	c-api/wg_c_gfxutil.h
	c-api/wg_c_gradient.cpp
	c-api/wg_c_gradient.h
	c-api/wg_c_internal.cpp
	c-api/wg_c_internal.h
	c-api/wg_c_patches.cpp
	c-api/wg_c_patches.h
	c-api/wg_c_printer.cpp
	c-api/wg_c_printer.h
	c-api/wg_c_surface.cpp
	c-api/wg_c_surface.h
	c-api/wg_c_surfacefactory.cpp
	c-api/wg_c_surfacefactory.h
	c-api/wg_c_surfacereader.cpp
	c-api/wg_c_surfacereader.h
	c-api/wg_c_surfacewriter.cpp
	c-api/wg_c_surfacewriter.h
	c-api/wg_c_edgemap.cpp
	c-api/wg_c_edgemap.h
	c-api/wg_c_edgemapfactory.cpp
	c-api/wg_c_edgemapfactory.h
	c-api/wondergfx_c.h
)

set(INCDIRS
	${WONDERGEAR_INCDIRS}
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/c-api
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wondergfx STATIC ${SOURCES} )
target_include_directories(wondergfx PRIVATE ${INCDIRS} )
target_link_libraries(wondergfx PUBLIC wondergear )


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WONDERGFX_SOURCES ${SOURCES} PARENT_SCOPE)
set(WONDERGFX_INCDIRS ${INCDIRS} PARENT_SCOPE)


