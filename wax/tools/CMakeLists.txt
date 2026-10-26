add_subdirectory(image2surf)
add_subdirectory(streamgenerator)
add_subdirectory(streamplayer)


set(TOOL_TARGETS
	image2surf
	streamgenerator
	streamplayer
)

foreach(TOOL ${TOOL_TARGETS})
	if(TARGET ${TOOL})
		set_target_properties(${TOOL} PROPERTIES FOLDER "Wax Tools")
	endif()
endforeach()
