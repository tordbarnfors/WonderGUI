

add_subdirectory(fontgenerator)
add_subdirectory(gfxdevicetester)
add_subdirectory(gfxworkbench)
add_subdirectory(gfxworkbench-gen2)
add_subdirectory(helloworld)
add_subdirectory(image2surf)
add_subdirectory(packlayouttester)
add_subdirectory(pluginworkbench)
add_subdirectory(softkernelgenerator)
add_subdirectory(surfviewer)
add_subdirectory(streamanalyzer)
add_subdirectory(streamgenerator)
add_subdirectory(streamplayer)
add_subdirectory(streamtrimmer)
add_subdirectory(texteditor)
add_subdirectory(unittester)
add_subdirectory(workbench)
add_subdirectory(legacyworkbench)
add_subdirectory(lineargfxworkbench)
add_subdirectory(embeddedworkbench)
add_subdirectory(legacyrunner)
add_subdirectory(layerworkbench)
add_subdirectory(widgetgallery)


set_target_properties(
	gfxdevicetester
	embeddedworkbench
	legacyworkbench
	legacytestplugin
	lineargfxworkbench
	testplugin
	fontgenerator
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
	streamtrimmer
	surfviewer
	texteditor
	unittester
	workbench
	legacyrunner
	layerworkbench
	widgetgallery
	PROPERTIES FOLDER "Tools" )
