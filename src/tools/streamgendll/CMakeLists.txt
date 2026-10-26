set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_VISIBILITY_PRESET invisible)
set(CMAKE_CPP_VISIBILITY_PRESET invisible)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

add_library(streamgendll MODULE

	${WAX_SOURCES}
	${WAX_SOURCES}
	${WAXGFXSTREAM_SOURCES}
	${WAXFREETYPEFONT_SOURCES}
	${WAXSOFTGFX_SOURCES}
)

generate_export_header(streamgendll BASE_NAME wg)

#set_target_properties(streamgendll PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

target_include_directories(streamgendll
	PRIVATE ${WAX_INCDIRS}
	PRIVATE ${WAX_INCDIRS}
	PRIVATE ${WAXGFXSTREAM_INCDIRS}
	PRIVATE ${WAXFREETYPEFONT_INCDIRS}
	PRIVATE ${WAXSOFTGFX_INCDIRS}
	PRIVATE ${FREETYPE2_INCDIR}

	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

target_link_directories(streamgendll PUBLIC
    ${FREETYPE2_LIBDIR}
)

target_link_libraries(streamgendll
	PRIVATE Freetype::Freetype		# Builds the FreeType font extension from source, so needs FreeType itself.
	PRIVATE ${FREETYPE2_LIB}
)

