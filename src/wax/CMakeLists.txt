set( SOURCES

	wax_backendlogger.cpp
	wax_backendlogger.h
	wax_core.cpp
	wax_core.h
	wax_blob.cpp
	wax_blob.h
	wax_blurbrush.cpp
	wax_blurbrush.h
	wax_canvaslayers.cpp
	wax_canvaslayers.h
	wax_chain.cpp
	wax_chain.h
	wax_color.cpp
	wax_color.h
	wax_componentptr.h
	wax_dataset.cpp
	wax_dataset.h
	wax_dynamicbuffer.h
	wax_enumextras.cpp
	wax_enumextras.h
	wax_geo.cpp
	wax_geo.h
	wax_gfxbackend.cpp
	wax_gfxbackend.h
	wax_gfxdevice.cpp
	wax_gfxdevice.h
	wax_gfxdevice_gen2.cpp
	wax_gfxdevice_gen2.h
	wax_gfxdevicefactory.cpp
	wax_gfxdevicefactory.h
	wax_gfxutil.cpp
	wax_gfxutil.h
	wax_gpushapes.cpp
	wax_gpushapes.h
	wax_memheap.cpp
	wax_memheap.h
	wax_mempool.cpp
	wax_mempool.h
	wax_memstack.cpp
	wax_memstack.h
	wax_object.cpp
	wax_object.h
	wax_patches.cpp
	wax_patches.h
	wax_patches.impl.h
	wax_pixeltools.cpp
	wax_pixeltools.h
	wax_pointers.h
	wax_printer.cpp
	wax_printer.h
	wax_spans.h
	wax_strongptr.h
	wax_tint.cpp
	wax_tint.h
	wax_tinttools.cpp
	wax_tinttools.h
	wax_transitions.cpp
	wax_transitions.h
	wax_types.h
	wax_weakptr.h

	compression/wax_compression.cpp
	compression/wax_compression.h
	compression/wax_lzcompression.cpp
	compression/wax_lzcompression.h
	compression/wax_q565compression.cpp
	compression/wax_q565compression.h
	compression/wax_qoicompression.cpp
	compression/wax_qoicompression.h
	compression/wax_rlecompression.cpp
	compression/wax_rlecompression.h
	compression/wax_spxcompression.cpp
	compression/wax_spxcompression.h

	edgemap/wax_edgemap.cpp
	edgemap/wax_edgemap.h
	edgemap/wax_edgemapfactory.cpp
	edgemap/wax_edgemapfactory.h
	edgemap/wax_edgemaptools.cpp
	edgemap/wax_edgemaptools.h
	edgemap/wax_waveform.cpp
	edgemap/wax_waveform.h

	font/wax_bitmapcache.cpp
	font/wax_bitmapcache.h
	font/wax_bitmapfont.cpp
	font/wax_bitmapfont.h
	font/wax_dummyfont.cpp
	font/wax_dummyfont.h
	font/wax_font.cpp
	font/wax_font.h
	font/wax_gridwalker.cpp
	font/wax_gridwalker.h
	font/wax_sysfont.cpp
	font/wax_sysfont.h

	surface/wax_surface.cpp
	surface/wax_surface.h
	surface/wax_surfacefactory.cpp
	surface/wax_surfacefactory.h
	surface/wax_surfacefileheader.h
	surface/wax_surfacefileinspector.cpp
	surface/wax_surfacefileinspector.h
	surface/wax_surfacereader.cpp
	surface/wax_surfacereader.h
	surface/wax_surfacewriter.cpp
	surface/wax_surfacewriter.h

	warp/wax_circularwarp.cpp
	warp/wax_circularwarp.h
	warp/wax_warpmap.cpp
	warp/wax_warpmap.h
	warp/wax_warpmapfactory.cpp
	warp/wax_warpmapfactory.h

	c-api/wax_c.h
	c-api/wax_c_core.cpp
	c-api/wax_c_core.h
	c-api/wax_c_bitmapcache.cpp
	c-api/wax_c_bitmapcache.h
	c-api/wax_c_bitmapfont.cpp
	c-api/wax_c_bitmapfont.h
	c-api/wax_c_blob.cpp
	c-api/wax_c_blob.h
	c-api/wax_c_blurbrush.cpp
	c-api/wax_c_blurbrush.h
	c-api/wax_c_canvaslayers.cpp
	c-api/wax_c_canvaslayers.h
	c-api/wax_c_circularwarp.cpp
	c-api/wax_c_circularwarp.h
	c-api/wax_c_color.cpp
	c-api/wax_c_color.h
	c-api/wax_c_edgemap.cpp
	c-api/wax_c_edgemap.h
	c-api/wax_c_edgemapfactory.cpp
	c-api/wax_c_edgemapfactory.h
	c-api/wax_c_font.cpp
	c-api/wax_c_font.h
	c-api/wax_c_geo.cpp
	c-api/wax_c_geo.h
	c-api/wax_c_gfxbackend.cpp
	c-api/wax_c_gfxbackend.h
	c-api/wax_c_gfxdevice.cpp
	c-api/wax_c_gfxdevice.h
	c-api/wax_c_gfxutil.cpp
	c-api/wax_c_gfxutil.h
	c-api/wax_c_internal.cpp
	c-api/wax_c_internal.h
	c-api/wax_c_memheap.cpp
	c-api/wax_c_memheap.h
	c-api/wax_c_object.cpp
	c-api/wax_c_object.h
	c-api/wax_c_patches.cpp
	c-api/wax_c_patches.h
	c-api/wax_c_printer.cpp
	c-api/wax_c_printer.h
	c-api/wax_c_surface.cpp
	c-api/wax_c_surface.h
	c-api/wax_c_surfacefactory.cpp
	c-api/wax_c_surfacefactory.h
	c-api/wax_c_surfacereader.cpp
	c-api/wax_c_surfacereader.h
	c-api/wax_c_surfacewriter.cpp
	c-api/wax_c_surfacewriter.h
	c-api/wax_c_sysfont.cpp
	c-api/wax_c_sysfont.h
	c-api/wax_c_tint.cpp
	c-api/wax_c_tint.h
	c-api/wax_c_types.cpp
	c-api/wax_c_types.h
	c-api/wax_c_warpmap.cpp
	c-api/wax_c_warpmap.h
	c-api/wax_c_warpmapfactory.cpp
	c-api/wax_c_warpmapfactory.h

	wax.h
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/compression
	${CMAKE_CURRENT_LIST_DIR}/edgemap
	${CMAKE_CURRENT_LIST_DIR}/font
	${CMAKE_CURRENT_LIST_DIR}/surface
	${CMAKE_CURRENT_LIST_DIR}/warp
	${CMAKE_CURRENT_LIST_DIR}/c-api
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wax STATIC ${SOURCES} )
generate_export_header(wax BASE_NAME wax)

target_include_directories(wax PRIVATE ${INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WAX_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAX_INCDIRS ${INCDIRS} PARENT_SCOPE)
