set(CMAKE_CXX_STANDARD 20)


set( SOURCES

	main.cpp
	stb_image.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(image2surf
	${SOURCES}
)


target_working_directory(image2surf ${WG_ROOT} )

target_link_libraries(image2surf PRIVATE
	wax
	waxsoftgfx
)

target_include_directories(image2surf PRIVATE
	${WAX_INCDIRS}
	${WAXSOFTGFX_INCDIRS}
)

