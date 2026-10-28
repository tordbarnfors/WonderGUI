add_subdirectory(streamgendll)
add_subdirectory(unittester)
add_subdirectory(embeddedworkbench)
add_subdirectory(workbench)

# Renders through OpenGL itself rather than through the WonderApp framework,
# so it is only built when the OpenGL extension is.

if(TARGET wglegacygui AND TARGET waxopenglgfx)
	add_subdirectory(legacyworkbench)
endif()
