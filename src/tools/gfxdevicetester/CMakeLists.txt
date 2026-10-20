
set(CMAKE_CXX_STANDARD 20)


set( SOURCES

	gfxdevicetester.cpp
	gfxdevicetester.h

	testsuites/a8tests.h
	testsuites/blurtests.h
	testsuites/blursourcetests.h
	testsuites/blendfixedcolortests.h
	testsuites/blendtests.h
	testsuites/blitblendtests.h
	testsuites/blitconsistencytest.h
	testsuites/blittests.h
	testsuites/canvasformattests.h
	testsuites/canvaslayertests.h
	testsuites/colortests.h
	testsuites/edgemaptests.h
	testsuites/edgemaptinttests.h
	testsuites/roundedboxtests.h
	testsuites/ringtests.h
	testsuites/warptests.h
	testsuites/linecaptests.h
	testsuites/edgemapringtests.h
	testsuites/edgemapwavetests.h
	testsuites/filltests.h
	testsuites/flattenlayerstests.h
	testsuites/layerroundtriptests.h
	testsuites/blendruletests.h
	testsuites/linetests.h
	testsuites/mipmaptests.h
	testsuites/paletteblittests.h
	testsuites/palettecapacitytests.h
	testsuites/piecharttests.h
	testsuites/readbacktests.h
	testsuites/rgb565bigendiantests.h
	testsuites/samplingtests.h
	testsuites/segmenttests.h
	testsuites/testsuite.h
	testsuites/tiletests.h
	testsuites/tintblittests.h
	testsuites/tintsegmenttests.h
	testsuites/tinttests.h
	testsuites/wavetests.h
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(gfxdevicetester
	${SOURCES}
)


target_include_directories(gfxdevicetester PRIVATE
	${WAPP_INCDIRS}
	${WGSOFTGFX_INCDIRS}
)

target_working_directory(gfxdevicetester ${WG_ROOT} )

target_link_libraries(gfxdevicetester PRIVATE
	${WAPPLIB}
	wgsoftgfx
)
