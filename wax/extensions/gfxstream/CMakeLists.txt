
set( SOURCES

	wax_gfxstream.cpp
	wax_gfxstream.h
	wax_mirrorsurfacefactory.cpp
	wax_mirrorsurfacefactory.h
	wax_mirrorsurface.cpp
	wax_mirrorsurface.h
	wax_remotesurface.cpp
	wax_remotesurface.h
	wax_remotesurfacefactory.cpp
	wax_remotesurfacefactory.h
	wax_streambackend.cpp
	wax_streambackend.h
	wax_streambuffer.cpp
	wax_streambuffer.h
	wax_streamdecoder.cpp
	wax_streamdecoder.h
	wax_streamencoder.cpp
	wax_streamencoder.h
	wax_streamgfxdevicefactory.cpp
	wax_streamgfxdevicefactory.h	
	wax_streamlogger.cpp
	wax_streamlogger.h
	wax_streamloopwrapper.cpp
	wax_streamloopwrapper.h
	wax_streammerger.cpp
	wax_streammerger.h
	wax_streamplayer.cpp
	wax_streamplayer.h
	wax_streampump.cpp
	wax_streampump.h
	wax_streamreader.cpp
	wax_streamreader.h
	wax_streamsink.h
	wax_streamsource.h
	wax_streamsplitter.cpp
	wax_streamsplitter.h
	wax_streamtrimbackend.cpp
	wax_streamtrimbackend.h
	wax_streamedgemap.cpp
	wax_streamedgemap.h
	wax_streamedgemapfactory.cpp
	wax_streamedgemapfactory.h
	wax_streamwarpmap.cpp
	wax_streamwarpmap.h
	wax_streamwarpmapfactory.cpp
	wax_streamwarpmapfactory.h
	wax_streamwrapper.cpp
	wax_streamwrapper.h
	wax_streamwriter.cpp
	wax_streamwriter.h
	waxgfxstream.h

	c-api/wax_c_streambuffer.h
	c-api/wax_c_streambuffer.cpp
	c-api/wax_c_streamencoder.h
	c-api/wax_c_streamencoder.cpp
	c-api/wax_c_streamgfx.h
	c-api/wax_c_streamgfx.cpp
	c-api/wax_c_streamlogger.cpp
	c-api/wax_c_streamlogger.h
	c-api/wax_c_streamloopwrapper.cpp
	c-api/wax_c_streamloopwrapper.h
	c-api/wax_c_streamplayer.cpp
	c-api/wax_c_streamplayer.h
	c-api/wax_c_streampump.cpp
	c-api/wax_c_streampump.h
	c-api/wax_c_streamreader.cpp
	c-api/wax_c_streamreader.h
	c-api/wax_c_streamsink.cpp
	c-api/wax_c_streamsink.h
	c-api/wax_c_streamsource.cpp
	c-api/wax_c_streamsource.h
	c-api/wax_c_streamsplitter.cpp
	c-api/wax_c_streamsplitter.h
	c-api/wax_c_streamtrimbackend.cpp
	c-api/wax_c_streamtrimbackend.h
	c-api/wax_c_streamwrapper.cpp
	c-api/wax_c_streamwrapper.h
	c-api/wax_c_streamwriter.cpp
	c-api/wax_c_streamwriter.h
	c-api/waxgfxstream_c.h
)

set(INCDIRS
	${WAX_INCDIRS}
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/c-api
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( waxgfxstream STATIC ${SOURCES} )
generate_export_header(waxgfxstream BASE_NAME wax)

target_include_directories(waxgfxstream PRIVATE ${INCDIRS} 
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)
target_link_libraries(waxgfxstream PUBLIC wax )		# Which brings wax.

set_target_properties( waxgfxstream PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WAXGFXSTREAM_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAXGFXSTREAM_INCDIRS ${INCDIRS} PARENT_SCOPE)


