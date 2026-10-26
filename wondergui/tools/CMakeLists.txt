add_subdirectory(compressbench)
add_subdirectory(editera)
add_subdirectory(fontgenerator)
add_subdirectory(gfxdevicetester)
add_subdirectory(helloworld)
add_subdirectory(packlayouttester)
add_subdirectory(softkernelgenerator)
add_subdirectory(surfviewer)
add_subdirectory(streamanalyzer)
add_subdirectory(streamgendll)
add_subdirectory(unittester)
add_subdirectory(widgetbench)
add_subdirectory(embeddedworkbench)

# These render through OpenGL themselves rather than through the WonderApp
# framework, so they are only built when the OpenGL extension is.

if(TARGET waxopenglgfx)
	add_subdirectory(workbench)
endif()

# These need extensions that are currently left out of the build.

if(TARGET wgplugin)
	add_subdirectory(pluginworkbench)
endif()

if(TARGET wglegacygui)
	add_subdirectory(legacyrunner)

	if(TARGET waxopenglgfx)
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
	testplugin
	fontgenerator
	helloworld
	packlayouttester
	pluginworkbench
	softkernelgenerator
	streamanalyzer
	streamgendll
	surfviewer
	unittester
	workbench
	widgetbench
	legacyrunner
)

foreach(TOOL ${TOOL_TARGETS})
	if(TARGET ${TOOL})
		set_target_properties(${TOOL} PROPERTIES FOLDER "Tools")
	endif()
endforeach()
