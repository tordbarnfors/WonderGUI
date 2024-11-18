source_group(Files REGULAR_EXPRESSION ".*")

			
set( SOURCES
	wg_legacyrootcapsule.cpp
	wg_legacyrootcapsule.h
	wg2_anim.cpp
	wg2_anim.h
	wg2_animplayer.cpp
	wg2_animplayer.h
	wg2_base.cpp
	wg2_base.h
	wg2_blockset.cpp
	wg2_blockset.h
	wg2_button.cpp
	wg2_button.h
	wg2_canvascapsule.cpp
	wg2_canvascapsule.h
	wg2_capsule.cpp
	wg2_capsule.h
	wg2_chart.cpp
	wg2_chart.h
	wg2_chart2.cpp
	wg2_chart2.h
	wg2_checkbox.cpp
	wg2_checkbox.h
	wg2_color.h
	wg2_colorset.cpp
	wg2_colorset.h
	wg2_container.cpp
	wg2_container.h
	wg2_cursor.cpp
	wg2_cursor.h
	wg2_cursorinstance.cpp
	wg2_cursorinstance.h
	wg2_dragndroplayer.cpp
	wg2_dragndroplayer.h
	wg2_event.cpp
	wg2_event.h
	wg2_eventfilter.cpp
	wg2_eventfilter.h
	wg2_eventhandler.cpp
	wg2_eventhandler.h
	wg2_eventlogger.cpp
	wg2_eventlogger.h
	wg2_filler.cpp
	wg2_filler.h
	wg2_flexpanel.cpp
	wg2_flexpanel.h
	wg2_flowpanel.cpp
	wg2_flowpanel.h
	wg2_fpsdisplay.cpp
	wg2_fpsdisplay.h
	wg2_geo.h
	wg2_gfxanim.cpp
	wg2_gfxanim.h
	wg2_gfxdevice.cpp
	wg2_gfxdevice.h
	wg2_hook.cpp
	wg2_hook.h
	wg2_iconholder.cpp
	wg2_iconholder.h
	wg2_image.cpp
	wg2_image.h
	wg2_interface_edittext.h
	wg2_interface_textholder.cpp
	wg2_interface_textholder.h
	wg2_key.h
	wg2_knob.cpp
	wg2_knob.h
	wg2_layer.cpp
	wg2_layer.h
	wg2_lineeditor.cpp
	wg2_lineeditor.h
	wg2_modallayer.cpp
	wg2_modallayer.h
	wg2_mouseblocker.cpp
	wg2_mouseblocker.h
	wg2_multislider.cpp
	wg2_multislider.h
	wg2_packpanel.cpp
	wg2_packpanel.h
	wg2_panel.cpp
	wg2_panel.h
	wg2_patches.cpp
	wg2_patches.h
	wg2_pen.cpp
	wg2_pen.h
	wg2_pianokeyboard.cpp
	wg2_pianokeyboard.h
	wg2_popuplayer.cpp
	wg2_popuplayer.h
	wg2_popupopener.cpp
	wg2_popupopener.h
	wg2_radiobutton.cpp
	wg2_radiobutton.h
	wg2_rangeslider.cpp
	wg2_rangeslider.h
	wg2_renderlayercapsule.cpp
	wg2_renderlayercapsule.h
	wg2_root.h
	wg2_rootpanel.cpp
	wg2_rootpanel.h
	wg2_rulerlabels.cpp
	wg2_rulerlabels.h
	wg2_scaleimage.cpp
	wg2_scaleimage.h
	wg2_scrollchart.cpp
	wg2_scrollchart.h
	wg2_scrollpanel.cpp
	wg2_scrollpanel.h
	wg2_shadercapsule.cpp
	wg2_shadercapsule.h
	wg2_shadowlayer.cpp
	wg2_shadowlayer.h
	wg2_simplevolumemeter.cpp
	wg2_simplevolumemeter.h
	wg2_sizebroker.cpp
	wg2_sizebroker.h
	wg2_sizecapsule.cpp
	wg2_sizecapsule.h
	wg2_slider.cpp
	wg2_slider.h
	wg2_slidertarget.cpp
	wg2_slidertarget.h
	wg2_smartptr.cpp
	wg2_smartptr.h
	wg2_stackpanel.cpp
	wg2_stackpanel.h
	wg2_switch.cpp
	wg2_switch.h
	wg2_text.cpp
	wg2_text.h
	wg2_textdisplay.cpp
	wg2_textdisplay.h
	wg2_tooltiplayer.cpp
	wg2_tooltiplayer.h
	wg2_tuner.h
	wg2_tuner.cpp
	wg2_types.h
	wg2_userdefines.h
	wg2_util.cpp
	wg2_util.h
	wg2_valueformat.cpp
	wg2_valueformat.h
	wg2_vectorpanel.cpp
	wg2_vectorpanel.h
	wg2_volumemeter.cpp
	wg2_volumemeter.h
	wg2_widget.cpp
	wg2_widget.h
	wg2_widgetholder.cpp
	wg2_widgetholder.h
	wg2_zoomoutcapsule.cpp
	wg2_zoomoutcapsule.h

	wglegacygui.h
)

set(INCDIRS
	${WONDERGUI_INCDIRS}
	${GFXSTREAM_INCDIRS}
	${WGSOFTGFX_INCDIRS}
	${WGFREETYPEFONT_INCDIRS}
	${CMAKE_CURRENT_LIST_DIR}
)


add_library( wglegacygui STATIC ${SOURCES} )
target_include_directories(wglegacygui PRIVATE ${INCDIRS} )
target_link_libraries(wglegacygui PUBLIC wondergui )

set_target_properties( wglegacygui PROPERTIES FOLDER "Extensions" )


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGLEGACYGUI_SOURCES ${SOURCES} PARENT_SCOPE)
set(WGLEGACYGUI_INCDIRS ${INCDIRS} PARENT_SCOPE)
