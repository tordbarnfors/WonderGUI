set(CMAKE_CXX_STANDARD 20)


set( SOURCES
	packlayouttester.cpp
	packlayouttester.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(packlayouttester
	${SOURCES}
)

target_include_directories(packlayouttester PRIVATE 
	${WAPP_INCDIRS}
)

target_working_directory(packlayouttester ${WG_ROOT} )

target_link_libraries(packlayouttester PRIVATE
	${WAPPLIB}
)

