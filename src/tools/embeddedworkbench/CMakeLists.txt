set(CMAKE_CXX_STANDARD 11)


# target_compile_options(wg-display PRIVATE -fno-exceptions -fno-rtti)


set( SOURCES
	cpp_necessities.cpp
	cpp_necessities.h
	main.c
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(embeddedworkbench
	${SOURCES}
)
generate_export_header(wondergfx BASE_NAME wg)


target_compile_options(embeddedworkbench PRIVATE -lm -lstd=c99)

target_working_directory(embeddedworkbench ${CMAKE_CURRENT_LIST_DIR} )

target_include_directories(embeddedworkbench PRIVATE
	${WONDERGFX_INCDIRS}
	${WGSOFTGFX_INCDIRS}
	${SDL2_INCDIR}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

target_link_directories(embeddedworkbench PRIVATE 
	${SDL2_LIBDIR}
)

target_link_libraries(embeddedworkbench PRIVATE

	wondergfx
	wgsoftgfx	
	${SDL2_LIBS}
)



