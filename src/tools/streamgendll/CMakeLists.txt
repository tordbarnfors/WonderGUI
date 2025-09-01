set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_VISIBILITY_PRESET invisible)
set(CMAKE_CPP_VISIBILITY_PRESET invisible)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

add_library(streamgendll MODULE
	dummy.cpp
)

target_link_directories(streamgendll PUBLIC
    ${FREETYPE2_LIBDIR}
)

target_link_libraries(streamgendll
	"$<LINK_LIBRARY:WHOLE_ARCHIVE,wondergfx,wggfxstream,wgfreetypefont>"
    ${FREETYPE2_LIB}
)



