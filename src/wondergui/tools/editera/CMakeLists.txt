set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	editera.cpp
	editera.h
	editorwindow.cpp
	editorwindow.h
	textbuffer.cpp
	textbuffer.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(editera
	${SOURCES}
)

target_include_directories(editera PRIVATE 
	${WAPP_INCDIRS}
)

target_working_directory(editera ${WG_ROOT} )

target_link_libraries(editera PRIVATE
	${WAPPLIB}
)


