add_subdirectory(compressbench)
add_subdirectory(editera)
add_subdirectory(fontgenerator)
add_subdirectory(gfxdevicetester)
add_subdirectory(packlayouttester)
add_subdirectory(softkernelgenerator)
add_subdirectory(surfviewer)
add_subdirectory(streamanalyzer)
add_subdirectory(widgetbench)

# These need extensions that are currently left out of the build.

if(TARGET wgplugin)
	add_subdirectory(pluginworkbench)
endif()

if(TARGET wglegacygui)
	add_subdirectory(legacyrunner)
endif()


set(TOOL_TARGETS
	compressbench
	editera
	fontgenerator
	gfxdevicetester
	legacyrunner
	legacytestplugin
	packlayouttester
	pluginworkbench
	softkernelgenerator
	streamanalyzer
	surfviewer
	testplugin
	widgetbench
)

foreach(TOOL ${TOOL_TARGETS})
	if(TARGET ${TOOL})
		set_target_properties(${TOOL} PROPERTIES FOLDER "WonderApp Tools")
	endif()
endforeach()
