set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	main.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(streamgenerator
	${SOURCES}
)

target_include_directories(streamgenerator PRIVATE
	${WAX_INCDIRS}
	${WAXSOFTGFX_INCDIRS}
	${WAXGFXSTREAM_INCDIRS}
	${STB_INCDIR}
)

target_working_directory(streamgenerator ${WG_ROOT} )

target_link_libraries(streamgenerator PRIVATE
	waxsoftgfx
	waxgfxstream
	${SDL3_LIBS}
)
