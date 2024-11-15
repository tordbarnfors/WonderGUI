
set( SOURCES

	wg_gfxstream.h
	wg_streambackend.cpp
	wg_streambackend.h
	wg_streambuffer.cpp
	wg_streambuffer.h
	wg_streamdecoder.cpp
	wg_streamdecoder.h
	wg_streamdevice.cpp
	wg_streamdevice.h
	wg_streamencoder.cpp
	wg_streamencoder.h
	wg_streamfastencoder.cpp
	wg_streamfastencoder.h
	wg_streamlogger.cpp
	wg_streamlogger.h
	wg_streamloopwrapper.cpp
	wg_streamloopwrapper.h
	wg_streamplayer.cpp
	wg_streamplayer.h
	wg_streampump.cpp
	wg_streampump.h
	wg_streamreader.cpp
	wg_streamreader.h
	wg_streamsink.h
	wg_streamsource.h
	wg_streamsplitter.cpp
	wg_streamsplitter.h
	wg_streamsurface.cpp
	wg_streamsurface.h
	wg_streamsurfacefactory.cpp
	wg_streamsurfacefactory.h
	wg_streamtrimencoder.cpp
	wg_streamtrimencoder.h	
	wg_streamedgemap.cpp
	wg_streamedgemap.h
	wg_streamedgemapfactory.cpp
	wg_streamedgemapfactory.h
	wg_streamwrapper.cpp
	wg_streamwrapper.h
	wg_streamwriter.cpp
	wg_streamwriter.h
	wondergfxstream.h

	c-api/wg_c_streambuffer.h
	c-api/wg_c_streambuffer.cpp
	c-api/wg_c_streamlogger.cpp
	c-api/wg_c_streamlogger.h
	c-api/wg_c_streamloopwrapper.cpp
	c-api/wg_c_streamloopwrapper.h
	c-api/wg_c_streamplayer.cpp
	c-api/wg_c_streamplayer.h
	c-api/wg_c_streampump.cpp
	c-api/wg_c_streampump.h
	c-api/wg_c_streamreader.cpp
	c-api/wg_c_streamreader.h
	c-api/wg_c_streamsink.cpp
	c-api/wg_c_streamsink.h
	c-api/wg_c_streamsource.cpp
	c-api/wg_c_streamsource.h
	c-api/wg_c_streamsplitter.cpp
	c-api/wg_c_streamsplitter.h
	c-api/wg_c_streamwrapper.cpp
	c-api/wg_c_streamwrapper.h
	c-api/wg_c_streamwriter.cpp
	c-api/wg_c_streamwriter.h
	c-api/wondergfxstream_c.h
)

set(INCDIRS
	${WONDERGEAR_INCDIRS}
	${WONDERGFX_INCDIRS}
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/c-api
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wggfxstream STATIC ${SOURCES} )
target_include_directories(wggfxstream PRIVATE ${INCDIRS} )
target_link_libraries(wggfxstream PUBLIC wondergear )

set_target_properties( wggfxstream PROPERTIES FOLDER "Extensions" )

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(GFXSTREAM_SOURCES ${SOURCES} PARENT_SCOPE)
set(GFXSTREAM_INCDIRS ${INCDIRS} PARENT_SCOPE)


