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
generate_export_header(wax BASE_NAME wg)


target_compile_options(embeddedworkbench PRIVATE -lm -lstd=c99)

target_working_directory(embeddedworkbench ${CMAKE_CURRENT_LIST_DIR} )

target_include_directories(embeddedworkbench PRIVATE
	${WAX_INCDIRS}
	${WAXSOFTGFX_INCDIRS}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

target_link_libraries(embeddedworkbench PRIVATE

	waxsoftgfx	
	wax
	${SDL3_LIBS}
)



