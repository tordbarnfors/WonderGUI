set(CMAKE_CXX_STANDARD 20)

# Every .cpp file in tests/ is a part of the workbench, so a new test only
# needs a new file. CONFIGURE_DEPENDS makes the build notice new files.

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h)

set( SOURCES
	main.cpp
	workbench.h
	workbench.cpp
	${TEST_SOURCES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(workbench
	${SOURCES}
)

target_working_directory(workbench ${WG_ROOT} )

target_include_directories(workbench PRIVATE
	${WONDERGUI_INCDIRS}
	${WAXSOFTGFX_INCDIRS}
	${WAXFREETYPEFONT_INCDIRS}
	${WGDEBUGGER_INCDIRS}
	${STB_INCDIR}
)

target_link_libraries(workbench PRIVATE
	wgdebugger
	waxsoftgfx
	waxfreetypefont
	wondergui
	${SDL3_LIBS}
)
