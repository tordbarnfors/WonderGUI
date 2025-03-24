set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	main.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(atariwg
	${SOURCES}
)

target_working_directory(atariwg ${WG_ROOT} )


target_include_directories(atariwg PRIVATE 
	${WONDERGUI_INCDIRS}
	${WGSOFTGFX_INCDIRS}
)

target_link_libraries(atariwg PRIVATE
	wondergui
	wgsoftgfx
)

