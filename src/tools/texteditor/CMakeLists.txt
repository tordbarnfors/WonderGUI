set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	texteditor.cpp
	texteditor.h
	editorwindow.cpp
	editorwindow.h
	textbuffer.cpp
	textbuffer.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(texteditor
	${SOURCES}
)

target_include_directories(texteditor PRIVATE 
	${WAPP_INCDIRS}
)

target_working_directory(texteditor ${WG_ROOT} )

target_link_libraries(texteditor PRIVATE
	${WAPPLIB}
)


