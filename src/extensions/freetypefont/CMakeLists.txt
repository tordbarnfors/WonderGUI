set(SOURCES
    wg_freetypefont.h
    wg_freetypefont.cpp
    wg_c_freetypefont.h
    wg_c_freetypefont.cpp
)


set(INCDIRS
   ${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wgfreetypefont STATIC ${SOURCES} )
generate_export_header(wgfreetypefont BASE_NAME wg)

target_include_directories(wgfreetypefont PRIVATE
    ${INCDIRS}
    ${WONDERGFX_INCDIRS}
    ${FREETYPE2_INCDIR}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

set_target_properties( wgfreetypefont PROPERTIES FOLDER "Extensions" )


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGFREETYPEFONT_SOURCES ${SOURCES} PARENT_SCOPE)
set(WGFREETYPEFONT_INCDIRS ${INCDIRS} PARENT_SCOPE)