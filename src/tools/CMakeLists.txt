add_subdirectory(compressbench)
add_subdirectory(editera)
add_subdirectory(fontgenerator)
add_subdirectory(gfxbench)
add_subdirectory(gfxdevicetester)
add_subdirectory(helloworld)
add_subdirectory(image2surf)
add_subdirectory(packlayouttester)
add_subdirectory(softkernelgenerator)
add_subdirectory(surfviewer)
add_subdirectory(streamanalyzer)
add_subdirectory(streamgenerator)
add_subdirectory(streamplayer)
add_subdirectory(streamgendll)
add_subdirectory(unittester)
add_subdirectory(widgetbench)
add_subdirectory(embeddedworkbench)
add_subdirectory(layerworkbench)

# These render through OpenGL themselves rather than through the WonderApp
# framework, so they are only built when the OpenGL extension is.

if(TARGET wgopenglgfx)
	add_subdirectory(gfxworkbench)
	add_subdirectory(gfxworkbench-gen2)
	add_subdirectory(workbench)
	add_subdirectory(lineargfxworkbench)
endif()

# These need extensions that are currently left out of the build.

if(TARGET wgplugin)
	add_subdirectory(pluginworkbench)
endif()

if(TARGET wglegacygui)
	add_subdirectory(legacyrunner)

	if(TARGET wgopenglgfx)
		add_subdirectory(legacyworkbench)
	endif()
endif()


set(TOOL_TARGETS
	compressbench
	editera
	gfxdevicetester
	embeddedworkbench
	legacyworkbench
	legacytestplugin
	lineargfxworkbench
	testplugin
	fontgenerator
	gfxbench
	gfxworkbench
	gfxworkbench-gen2
	helloworld
	image2surf
	packlayouttester
	pluginworkbench
	softkernelgenerator
	streamanalyzer
	streamgenerator
	streamplayer
	streamgendll
	surfviewer
	unittester
	workbench
	widgetbench
	legacyrunner
	layerworkbench
)

foreach(TOOL ${TOOL_TARGETS})
	if(TARGET ${TOOL})
		set_target_properties(${TOOL} PROPERTIES FOLDER "Tools")
	endif()
endforeach()
