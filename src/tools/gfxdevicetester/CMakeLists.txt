
set(CMAKE_CXX_STANDARD 20)


set( SOURCES

	device.cpp
	device.h
	gfxdevicetester.cpp
	gfxdevicetester.h
	testunit.h
	
	testsuites/a8tests.h
	testsuites/blurtests.h
	testsuites/blendfixedcolortests.h
	testsuites/blendtests.h
	testsuites/blitblendtests.h
	testsuites/blitconsistencytest.h
	testsuites/blittests.h
	testsuites/canvasformattests.h
	testsuites/canvaslayertests.h
	testsuites/colortests.h
	testsuites/edgemaptests.h
	testsuites/edgemaptoolstests.h
	testsuites/filltests.h
	testsuites/linetests.h
	testsuites/mipmaptests.h
	testsuites/paletteblittests.h
	testsuites/piecharttests.h
	testsuites/plottests.h
	testsuites/rgb565bigendiantests.h	
	testsuites/segmenttests.h
	testsuites/testsuite.h
	testsuites/tiletests.h
	testsuites/tintblittests.h
	testsuites/tintsegmenttests.h
	testsuites/wavetests.h
	testsuites/waveformtests.h
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(gfxdevicetester
	${SOURCES}
)


target_include_directories(gfxdevicetester PRIVATE
	${WAPP_INCDIRS}
	${WGOPENGLGFX_INCDIRS}
	${WGSOFTGFX_INCDIRS}
	${GFXSTREAM_INCDIRS}
)

target_link_directories(gfxdevicetester PRIVATE 
)

target_working_directory(gfxdevicetester ${WG_ROOT} )

target_link_libraries(gfxdevicetester PRIVATE
	${WAPPLIB}
	wgsoftgfx
	wgopenglgfx
	wggfxstream
)

