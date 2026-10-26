set(SOURCES
    wax_freetypefont.h
    wax_freetypefont.cpp
    wax_c_freetypefont.h
    wax_c_freetypefont.cpp
)


set(INCDIRS
   ${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( waxfreetypefont STATIC ${SOURCES} )
generate_export_header(waxfreetypefont BASE_NAME wax)

target_link_libraries( waxfreetypefont 
    wax
    Freetype::Freetype
)

target_include_directories(waxfreetypefont PRIVATE
    ${INCDIRS}
    ${WAX_INCDIRS}
    ${FREETYPE2_INCDIR}
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

set_target_properties( waxfreetypefont PROPERTIES FOLDER "Extensions" )


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WAXFREETYPEFONT_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAXFREETYPEFONT_INCDIRS ${INCDIRS} PARENT_SCOPE)