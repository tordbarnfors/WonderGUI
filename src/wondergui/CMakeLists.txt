

set( SOURCES
	base/wg_animframes.cpp
	base/wg_animframes.h
	base/wg_caret.cpp
	base/wg_caret.h
	base/wg_char.h
	base/wg_charbuffer.cpp
	base/wg_charbuffer.h
	base/wg_charseq.cpp
	base/wg_charseq.h
	base/wg_codepages.cpp
	base/wg_codepages.h
	base/wg_dynamicvector.h
	base/wg_enumextras.cpp
	base/wg_enumextras.h
	base/wg_base.cpp
	base/wg_base.h
	base/wg_hostbridge.h
	base/wg_inputhandler.cpp
	base/wg_inputhandler.h
	base/wg_key.h
	base/wg_msg.cpp
	base/wg_msg.h
	base/wg_msglogger.cpp
	base/wg_msglogger.h
	base/wg_msgrouter.cpp
	base/wg_msgrouter.h
	base/wg_packlayout.cpp
	base/wg_packlayout.h
	base/wg_receiver.cpp
	base/wg_receiver.h
	base/wg_root.cpp
	base/wg_root.h
	base/wg_skinslot.cpp
	base/wg_skinslot.h
	base/wg_skinslotmanager.cpp
	base/wg_skinslotmanager.h
	base/wg_skinslotmi.cpp
	base/wg_skinslotmi.h
	base/wg_state.cpp
	base/wg_state.h
	base/wg_staticvector.h
	base/wg_string.cpp
	base/wg_string.h
	base/wg_textlink.cpp
	base/wg_textlink.h
	base/wg_textstyle.cpp
	base/wg_textstyle.h
	base/wg_textstylemanager.cpp
	base/wg_textstylemanager.h
	base/wg_texttool.cpp
	base/wg_texttool.h
	base/wg_theme.h
	base/wg_theme.cpp	
	base/wg_togglegroup.cpp
	base/wg_togglegroup.h
	base/wg_types.h
	base/wg_util.cpp
	base/wg_util.h


	c-api/wg_c_hostbridge.cpp
	c-api/wg_c_hostbridge.h
	c-api/wg_c_types.h

	components/wg_columnheader.cpp
	components/wg_columnheader.h
	components/wg_editablenumber.cpp
	components/wg_editablenumber.h
	components/wg_editabletext.cpp
	components/wg_editabletext.h
	components/wg_glow.cpp
	components/wg_glow.h
	components/wg_icon.cpp
	components/wg_icon.h
	components/wg_number.cpp
	components/wg_number.h
	components/wg_scroller.cpp
	components/wg_scroller.h
	components/wg_staticnumber.cpp
	components/wg_staticnumber.h
	components/wg_statictext.cpp
	components/wg_statictext.h
	components/wg_text.cpp
	components/wg_text.h
	components/wg_textbase.cpp
	components/wg_textbase.h
	components/wg_component.cpp
	components/wg_component.h

	numberlayouts/wg_basicnumberlayout.cpp
	numberlayouts/wg_basicnumberlayout.h
	numberlayouts/wg_numberlayout.cpp
	numberlayouts/wg_numberlayout.h

	skins/wg_animskin.cpp
	skins/wg_animskin.h
	skins/wg_skin.cpp
	skins/wg_skin.h
	skins/wg_skin.impl.h
	skins/wg_stateskin.cpp
	skins/wg_stateskin.h	
	skins/anim/wg_spinanimskin.cpp
	skins/anim/wg_spinanimskin.h
	skins/meter/wg_fillmeterskin.cpp
	skins/meter/wg_fillmeterskin.h
	skins/meter/wg_framemeterskin.cpp
	skins/meter/wg_framemeterskin.h
	skins/meter/wg_piemeterskin.cpp
	skins/meter/wg_piemeterskin.h
	skins/meter/wg_spinmeterskin.cpp
	skins/meter/wg_spinmeterskin.h
	skins/misc/wg_bakeskin.cpp
	skins/misc/wg_bakeskin.h
	skins/misc/wg_lambdaskin.cpp
	skins/misc/wg_lambdaskin.h
	skins/misc/wg_doubleskin.cpp
	skins/misc/wg_doubleskin.h
	skins/state/wg_blockskin.cpp
	skins/state/wg_blockskin.h
	skins/state/wg_blockslideskin.cpp
	skins/state/wg_blockslideskin.h
	skins/state/wg_boxskin.cpp
	skins/state/wg_boxskin.h
	skins/state/wg_bracketskin.cpp
	skins/state/wg_bracketskin.h
	skins/state/wg_circleskin.cpp
	skins/state/wg_circleskin.h
	skins/state/wg_colorskin.cpp
	skins/state/wg_colorskin.h
#	skins/state/wg_gradientskin.cpp
#	skins/state/wg_gradientskin.h	
#	skins/state/wg_multiblockskin.cpp
#	skins/state/wg_multiblockskin.h
	skins/state/wg_tileskin.cpp
	skins/state/wg_tileskin.h

	slots/wg_slot.cpp
	slots/wg_slot.h
	slots/wg_slotextras.h
	slots/wg_slotholder.h
	slots/wg_dynamicslotvector.h
	slots/wg_dynamicslotvector.impl.h
	slots/wg_slotarray.h
	slots/wg_slottable.cpp
	slots/wg_slottable.h
	slots/wg_staticslotvector.h
	slots/wg_staticslotvector.impl.h

	textlayouts/wg_basictextlayout.cpp
	textlayouts/wg_basictextlayout.h
	textlayouts/wg_textlayout.cpp
	textlayouts/wg_textlayout.h

	widgets/wg_animplayer.cpp
	widgets/wg_animplayer.h
	widgets/wg_areachart.cpp
	widgets/wg_areachart.h
	widgets/wg_areascrollchart.cpp
	widgets/wg_areascrollchart.h
	widgets/wg_button.cpp
	widgets/wg_button.h
	widgets/wg_canvasdisplay.cpp
	widgets/wg_canvasdisplay.h
	widgets/wg_chart.cpp
	widgets/wg_chart.h
	widgets/wg_container.cpp
	widgets/wg_container.h
	widgets/wg_filler.cpp
	widgets/wg_filler.h
	widgets/wg_fpsdisplay.cpp
	widgets/wg_fpsdisplay.h
	widgets/wg_image.cpp
	widgets/wg_image.h
	widgets/wg_knob.cpp
	widgets/wg_knob.h
	widgets/wg_lineeditor.cpp
	widgets/wg_lineeditor.h
	widgets/wg_pianokeyboard.cpp
	widgets/wg_pianokeyboard.h
	widgets/wg_plotchart.cpp
	widgets/wg_plotchart.h
	widgets/wg_popupopener.cpp
	widgets/wg_popupopener.h
	widgets/wg_rangeslider.cpp
	widgets/wg_rangeslider.h
	widgets/wg_scrollbar.cpp
	widgets/wg_scrollbar.h
	widgets/wg_scrollchart.cpp
	widgets/wg_scrollchart.h
	widgets/wg_selectbox.cpp
	widgets/wg_selectbox.h
	widgets/wg_sidecanvas.cpp
	widgets/wg_sidecanvas.h
	widgets/wg_skindisplay.cpp
	widgets/wg_skindisplay.h
	widgets/wg_slider.cpp
	widgets/wg_slider.h
	widgets/wg_surfacedisplay.cpp
	widgets/wg_surfacedisplay.h
	widgets/wg_textdisplay.cpp
	widgets/wg_textdisplay.h
	widgets/wg_texteditor.cpp
	widgets/wg_texteditor.h
	widgets/wg_timer.cpp
	widgets/wg_timer.h
	widgets/wg_togglebutton.cpp
	widgets/wg_togglebutton.h
	widgets/wg_numberdisplay.cpp
	widgets/wg_numberdisplay.h
	widgets/wg_widget.cpp
	widgets/wg_widget.h

	widgets/capsules/wg_blockingcapsule.cpp
	widgets/capsules/wg_blockingcapsule.h
	widgets/capsules/wg_canvascapsule.cpp
	widgets/capsules/wg_canvascapsule.h
	widgets/capsules/wg_capsule.cpp
	widgets/capsules/wg_capsule.h
	widgets/capsules/wg_labelcapsule.cpp
	widgets/capsules/wg_labelcapsule.h
	widgets/capsules/wg_paddingcapsule.cpp
	widgets/capsules/wg_paddingcapsule.h
	widgets/capsules/wg_renderlayercapsule.cpp
	widgets/capsules/wg_renderlayercapsule.h
	widgets/capsules/wg_reordercapsule.cpp
	widgets/capsules/wg_reordercapsule.h
	widgets/capsules/wg_scalecapsule.cpp
	widgets/capsules/wg_scalecapsule.h
	widgets/capsules/wg_selectcapsule.cpp
	widgets/capsules/wg_selectcapsule.h
	widgets/capsules/wg_sizecapsule.cpp
	widgets/capsules/wg_sizecapsule.h

	widgets/overlays/wg_dragndropoverlay.cpp
	widgets/overlays/wg_dragndropoverlay.h
	widgets/overlays/wg_overlay.cpp
	widgets/overlays/wg_overlay.h
	widgets/overlays/wg_modaloverlay.cpp
	widgets/overlays/wg_modaloverlay.h
	widgets/overlays/wg_popupoverlay.cpp
	widgets/overlays/wg_popupoverlay.h
	widgets/overlays/wg_tooltipoverlay.cpp
	widgets/overlays/wg_tooltipoverlay.h

	widgets/lists/wg_list.cpp
	widgets/lists/wg_list.h
	widgets/lists/wg_packlist.cpp
	widgets/lists/wg_packlist.h
#	widgets/lists/wg_treelist.cpp
#	widgets/lists/wg_treelist.h

	widgets/panels/wg_drawerpanel.cpp
	widgets/panels/wg_drawerpanel.h
	widgets/panels/wg_flexpanel.cpp
	widgets/panels/wg_flexpanel.h
	widgets/panels/wg_lambdapanel.cpp
	widgets/panels/wg_lambdapanel.h
	widgets/panels/wg_packpanel.cpp
	widgets/panels/wg_packpanel.h
	widgets/panels/wg_panel.cpp
	widgets/panels/wg_panel.h
	widgets/panels/wg_panel.impl.h
	widgets/panels/wg_rootpanel.cpp
	widgets/panels/wg_rootpanel.h
	widgets/panels/wg_scrollpanel.cpp
	widgets/panels/wg_scrollpanel.h
	widgets/panels/wg_splitpanel.cpp
	widgets/panels/wg_splitpanel.h
	widgets/panels/wg_stackpanel.cpp
	widgets/panels/wg_stackpanel.h
	widgets/panels/wg_tablepanel.cpp
	widgets/panels/wg_tablepanel.h
	widgets/panels/wg_twoslotpanel.cpp
	widgets/panels/wg_twoslotpanel.h

	wondergui.h
)


set(INCDIRS
	${WONDERGFX_INCDIRS}
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/base/
	${CMAKE_CURRENT_LIST_DIR}/c-api/
	${CMAKE_CURRENT_LIST_DIR}/components/
	${CMAKE_CURRENT_LIST_DIR}/numberlayouts/
	${CMAKE_CURRENT_LIST_DIR}/skins/
	${CMAKE_CURRENT_LIST_DIR}/skins/anim/
	${CMAKE_CURRENT_LIST_DIR}/skins/meter/
	${CMAKE_CURRENT_LIST_DIR}/skins/misc/
	${CMAKE_CURRENT_LIST_DIR}/skins/state/
	${CMAKE_CURRENT_LIST_DIR}/skins/static/
	${CMAKE_CURRENT_LIST_DIR}/slots/
	${CMAKE_CURRENT_LIST_DIR}/textlayouts/
	${CMAKE_CURRENT_LIST_DIR}/widgets/
	${CMAKE_CURRENT_LIST_DIR}/widgets/capsules/
	${CMAKE_CURRENT_LIST_DIR}/widgets/lists/
	${CMAKE_CURRENT_LIST_DIR}/widgets/overlays/
	${CMAKE_CURRENT_LIST_DIR}/widgets/panels/
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wondergui STATIC ${SOURCES} )
target_include_directories(wondergui PRIVATE ${INCDIRS} )
target_link_libraries(wondergui PUBLIC wondergfx )

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WONDERGUI_SOURCES ${SOURCES} PARENT_SCOPE)
set(WONDERGUI_INCDIRS ${INCDIRS} PARENT_SCOPE)
