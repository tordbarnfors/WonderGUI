#------------------------------------------------------------------------------
# FreeType
#
# Only looked for here, so nothing needs it unless this extension is built.
# Code that compiles this extension's sources into its own target (such as
# streamgendll and the test plugins) links Freetype::Freetype as well, so the
# target is made visible to the whole build.
#------------------------------------------------------------------------------

find_package(Freetype QUIET)

if(Freetype_FOUND)
	set_target_properties(Freetype::Freetype PROPERTIES IMPORTED_GLOBAL TRUE)
elseif(WONDERGUI_FETCH_DEPS)
	include(FetchContent)
	FetchContent_Declare(
		freetype
		GIT_REPOSITORY https://github.com/freetype/freetype.git
		GIT_TAG        VER-2-14-3
	)
	block()
		set(CMAKE_FOLDER thirdparty)
		FetchContent_MakeAvailable(freetype)
	endblock()
	add_library(Freetype::Freetype ALIAS freetype)
endif()

if(NOT TARGET Freetype::Freetype)
	message(FATAL_ERROR "FreeType not found. Install it, or enable WONDERGUI_FETCH_DEPS to download it.")
endif()


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


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WAXFREETYPEFONT_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAXFREETYPEFONT_INCDIRS ${INCDIRS} PARENT_SCOPE)