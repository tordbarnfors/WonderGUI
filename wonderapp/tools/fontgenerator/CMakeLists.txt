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
	${FREETYPE2_INCDIR}

)

target_working_directory(fontgenerator ${WG_ROOT} )

target_link_libraries(fontgenerator PRIVATE
	${WAPPLIB}
)

