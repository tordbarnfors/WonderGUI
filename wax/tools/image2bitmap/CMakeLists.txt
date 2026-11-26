set(CMAKE_CXX_STANDARD 20)


set( SOURCES

	main.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(image2bitmap
	${SOURCES}
)


target_working_directory(image2bitmap ${WG_ROOT} )

target_link_libraries(image2bitmap PRIVATE
	wax
	waxsoftgfx
)

target_include_directories(image2bitmap PRIVATE
	${WAX_INCDIRS}
	${WAXSOFTGFX_INCDIRS}
	${STB_INCDIR}
)

