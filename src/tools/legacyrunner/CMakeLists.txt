set(CMAKE_CXX_STANDARD 20)


set( SOURCES
	legacyrunner.cpp
	legacyrunner.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(legacyrunner ${SOURCES})
generate_export_header(wondergfx BASE_NAME wg)

target_include_directories(legacyrunner PRIVATE 
	${WAPP_INCDIRS}
	${WGLEGACYGUI_INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

target_working_directory(legacyrunner ${WG_ROOT} )

target_link_libraries(legacyrunner PRIVATE
	${WAPPLIB}
	wglegacygui

)

