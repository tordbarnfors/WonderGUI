add_subdirectory(streamgendll)
add_subdirectory(unittester)
add_subdirectory(embeddedworkbench)

# These render through OpenGL themselves rather than through the WonderApp
# framework, so they are only built when the OpenGL extension is.

if(TARGET waxopenglgfx)
	add_subdirectory(workbench)
endif()

# These need extensions that are currently left out of the build.

if(TARGET wglegacygui AND TARGET waxopenglgfx)
	add_subdirectory(legacyworkbench)
endif()
