set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	main.cpp
	testframework.cpp
	testframework.h
	tests/gfxstream.cpp
	tests/gfxstream.h
	tests/softgfxdevice.cpp
	tests/softgfxdevice.h
	tests/slotlinks.cpp
	tests/slotlinks.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(unittester
	${SOURCES}
)

target_working_directory(unittester ${WG_ROOT} )
generate_export_header(wondergfx BASE_NAME wg)


target_include_directories(unittester PRIVATE 
	${WONDERGUI_INCDIRS}
	${WGSOFTGFX_INCDIRS}
	${WGGFXSTREAM_INCDIRS}
	${WGFREETYPEFONT_INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

target_link_libraries(unittester PRIVATE
	wgfreetypefont
	wondergui
	wgsoftgfx
	wggfxstream
)

