### This tool uses SDL2_image for generating PNG-files. Can only be built if
### SDL2_image development libs and headers are available and linked against
###a SDL2-based WonderApp framework.

set(CMAKE_CXX_STANDARD 20)


set( SOURCES
	fontgenerator.cpp
	fontgenerator.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(fontgenerator
	${SOURCES}
)


target_include_directories(fontgenerator PRIVATE 
	${WAPP_INCDIRS}
	${SDL2_INCDIR}
	${SDL2IMAGE_INCDIR}
	${FREETYPE2_INCDIR}

)

target_working_directory(fontgenerator ${WG_ROOT} )

target_link_libraries(fontgenerator PRIVATE
	${WAPPLIB}
)

