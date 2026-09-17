
set(CMAKE_CXX_STANDARD 20)

# Add each new test file here - this is the only place besides tests/*.cpp
# itself that adding a test touches. Tests self-register with TestRegistry
# (see test.h), so widgetbench.cpp never needs to change.
set( TEST_SOURCES

	tests/text_test.cpp
	tests/buttons_test.cpp
	tests/checkbox_test.cpp
	tests/gallery_test.cpp
	tests/scroll_list_test.cpp
)

set( SOURCES

	widgetbench.cpp
	widgetbench.h
	test.cpp
	test.h

	${TEST_SOURCES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(widgetbench
	${SOURCES}
)

target_include_directories(widgetbench PRIVATE 
	${WAPP_INCDIRS}
)

target_working_directory(widgetbench ${WG_ROOT} )

target_link_libraries(widgetbench PRIVATE
	${WAPPLIB}
)
