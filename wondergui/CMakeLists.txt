

set( SOURCES
	source/base/wg_animframes.cpp
	source/base/wg_animframes.h
	source/base/wg_caret.cpp
	source/base/wg_caret.h
	source/base/wg_char.h
	source/base/wg_charbuffer.cpp
	source/base/wg_charbuffer.h
	source/base/wg_charseq.cpp
	source/base/wg_charseq.h
	source/base/wg_codepages.cpp
	source/base/wg_codepages.h
	source/base/wg_dynamicvector.h
	source/base/wg_enumextras.cpp
	source/base/wg_enumextras.h
	source/base/wg_base.cpp
	source/base/wg_base.h
	source/base/wg_hostbridge.h
	source/base/wg_inputhandler.cpp
	source/base/wg_inputhandler.h
	source/base/wg_key.h
	source/base/wg_msg.cpp
	source/base/wg_msg.h
	source/base/wg_msglogger.cpp
	source/base/wg_msglogger.h
	source/base/wg_msgrouter.cpp
	source/base/wg_msgrouter.h
	source/base/wg_packlayout.cpp
	source/base/wg_packlayout.h
	source/base/wg_receiver.cpp
	source/base/wg_receiver.h
	source/base/wg_root.cpp
	source/base/wg_root.h
	source/base/wg_skinslot.cpp
	source/base/wg_skinslot.h
	source/base/wg_skinslotmanager.cpp
	source/base/wg_skinslotmanager.h
	source/base/wg_skinslotmi.cpp
	source/base/wg_skinslotmi.h
	source/base/wg_state.cpp
	source/base/wg_state.h
	source/base/wg_staticvector.h
	source/base/wg_string.cpp
	source/base/wg_string.h
	source/base/wg_textlink.cpp
	source/base/wg_textlink.h
	source/base/wg_textstyle.cpp
	source/base/wg_textstyle.h
	source/base/wg_textstylemanager.cpp
	source/base/wg_textstylemanager.h
	source/base/wg_texttool.cpp
	source/base/wg_texttool.h
	source/base/wg_togglegroup.cpp
	source/base/wg_togglegroup.h
	source/base/wg_types.h
	source/base/wg_wax.h
	source/base/wg_util.cpp
	source/base/wg_util.h

	source/components/wg_columnheader.cpp
	source/components/wg_columnheader.h
	source/components/wg_editablenumber.cpp
	source/components/wg_editablenumber.h
	source/components/wg_editabletext.cpp
	source/components/wg_editabletext.h
	source/components/wg_glow.cpp
	source/components/wg_glow.h
	source/components/wg_icon.cpp
	source/components/wg_icon.h
	source/components/wg_number.cpp
	source/components/wg_number.h
	source/components/wg_scroller.cpp
	source/components/wg_scroller.h
	source/components/wg_staticnumber.cpp
	source/components/wg_staticnumber.h
	source/components/wg_statictext.cpp
	source/components/wg_statictext.h
	source/components/wg_dynamictext.cpp
	source/components/wg_dynamictext.h
	source/components/wg_component.cpp
	source/components/wg_component.h

	source/numberlayouts/wg_basicnumberlayout.cpp
	source/numberlayouts/wg_basicnumberlayout.h
	source/numberlayouts/wg_numberlayout.cpp
	source/numberlayouts/wg_numberlayout.h

	source/skins/wg_animskin.cpp
	source/skins/wg_animskin.h
	source/skins/wg_skin.cpp
	source/skins/wg_skin.h
	source/skins/wg_skin.impl.h
	source/skins/wg_stateskin.cpp
	source/skins/wg_stateskin.h	
	source/skins/anim/wg_spinanimskin.cpp
	source/skins/anim/wg_spinanimskin.h
	source/skins/meter/wg_fillmeterskin.cpp
	source/skins/meter/wg_fillmeterskin.h
	source/skins/meter/wg_framemeterskin.cpp
	source/skins/meter/wg_framemeterskin.h
	source/skins/meter/wg_piemeterskin.cpp
	source/skins/meter/wg_piemeterskin.h
	source/skins/meter/wg_spinmeterskin.cpp
	source/skins/meter/wg_spinmeterskin.h
	source/skins/misc/wg_bakeskin.cpp
	source/skins/misc/wg_bakeskin.h
	source/skins/misc/wg_lambdaskin.cpp
	source/skins/misc/wg_lambdaskin.h
	source/skins/misc/wg_doubleskin.cpp
	source/skins/misc/wg_doubleskin.h
	source/skins/state/wg_blockskin.cpp
	source/skins/state/wg_blockskin.h
	source/skins/state/wg_blockslideskin.cpp
	source/skins/state/wg_blockslideskin.h
	source/skins/state/wg_boxskin.cpp
	source/skins/state/wg_boxskin.h
	source/skins/state/wg_bracketskin.cpp
	source/skins/state/wg_bracketskin.h
	source/skins/state/wg_circleskin.cpp
	source/skins/state/wg_circleskin.h
	source/skins/state/wg_colorskin.cpp
	source/skins/state/wg_colorskin.h
	source/skins/state/wg_multiblockskin.cpp
	source/skins/state/wg_multiblockskin.h
	source/skins/state/wg_tileskin.cpp
	source/skins/state/wg_tileskin.h
	source/skins/state/wg_tintskin.cpp
	source/skins/state/wg_tintskin.h


	source/slots/wg_slot.cpp
	source/slots/wg_slot.h
	source/slots/wg_slotextras.h
	source/slots/wg_slotholder.h
	source/slots/wg_dynamicslotvector.h
	source/slots/wg_dynamicslotvector.impl.h
	source/slots/wg_slotarray.h
	source/slots/wg_slottable.cpp
	source/slots/wg_slottable.h
	source/slots/wg_staticslotvector.h
	source/slots/wg_staticslotvector.impl.h

	source/textlayouts/wg_basictextlayout.cpp
	source/textlayouts/wg_basictextlayout.h
	source/textlayouts/wg_textlayout.cpp
	source/textlayouts/wg_textlayout.h

	source/widgets/wg_animplayer.cpp
	source/widgets/wg_animplayer.h
	source/widgets/wg_areachart.cpp
	source/widgets/wg_areachart.h
	source/widgets/wg_areascrollchart.cpp
	source/widgets/wg_areascrollchart.h
	source/widgets/wg_button.cpp
	source/widgets/wg_button.h
	source/widgets/wg_canvasdisplay.cpp
	source/widgets/wg_canvasdisplay.h
	source/widgets/wg_chart.cpp
	source/widgets/wg_chart.h
	source/widgets/wg_container.cpp
	source/widgets/wg_container.h
	source/widgets/wg_edgemapdisplay.cpp
	source/widgets/wg_edgemapdisplay.h
	source/widgets/wg_filler.cpp
	source/widgets/wg_filler.h
	source/widgets/wg_fpsdisplay.cpp
	source/widgets/wg_fpsdisplay.h
	source/widgets/wg_image.cpp
	source/widgets/wg_image.h
	source/widgets/wg_knob.cpp
	source/widgets/wg_knob.h
	source/widgets/wg_lineeditor.cpp
	source/widgets/wg_lineeditor.h
	source/widgets/wg_meter.cpp
	source/widgets/wg_meter.h	
	source/widgets/wg_pianokeyboard.cpp
	source/widgets/wg_pianokeyboard.h
	source/widgets/wg_plotchart.cpp
	source/widgets/wg_plotchart.h
	source/widgets/wg_popupopener.cpp
	source/widgets/wg_popupopener.h
	source/widgets/wg_rangeslider.cpp
	source/widgets/wg_rangeslider.h
	source/widgets/wg_scrollbar.cpp
	source/widgets/wg_scrollbar.h
	source/widgets/wg_scrollchart.cpp
	source/widgets/wg_scrollchart.h
	source/widgets/wg_selectbox.cpp
	source/widgets/wg_selectbox.h
	source/widgets/wg_sidecanvas.cpp
	source/widgets/wg_sidecanvas.h
	source/widgets/wg_skindisplay.cpp
	source/widgets/wg_skindisplay.h
	source/widgets/wg_slider.cpp
	source/widgets/wg_slider.h
	source/widgets/wg_surfacedisplay.cpp
	source/widgets/wg_surfacedisplay.h
	source/widgets/wg_textdisplay.cpp
	source/widgets/wg_textdisplay.h
	source/widgets/wg_texteditor.cpp
	source/widgets/wg_texteditor.h
	source/widgets/wg_timer.cpp
	source/widgets/wg_timer.h
	source/widgets/wg_togglebutton.cpp
	source/widgets/wg_togglebutton.h
	source/widgets/wg_nodewires.cpp
	source/widgets/wg_nodewires.h
	source/widgets/wg_numberdisplay.cpp
	source/widgets/wg_numberdisplay.h
	source/widgets/wg_widget.cpp
	source/widgets/wg_widget.h

	source/widgets/capsules/wg_blockingcapsule.cpp
	source/widgets/capsules/wg_blockingcapsule.h
	source/widgets/capsules/wg_canvascapsule.cpp
	source/widgets/capsules/wg_canvascapsule.h
	source/widgets/capsules/wg_capsule.cpp
	source/widgets/capsules/wg_capsule.h
	source/widgets/capsules/wg_labelcapsule.cpp
	source/widgets/capsules/wg_labelcapsule.h
	source/widgets/capsules/wg_paddingcapsule.cpp
	source/widgets/capsules/wg_paddingcapsule.h
	source/widgets/capsules/wg_renderlayercapsule.cpp
	source/widgets/capsules/wg_renderlayercapsule.h
	source/widgets/capsules/wg_reordercapsule.cpp
	source/widgets/capsules/wg_reordercapsule.h
	source/widgets/capsules/wg_scalecapsule.cpp
	source/widgets/capsules/wg_scalecapsule.h
	source/widgets/capsules/wg_scrollcapsule.cpp
	source/widgets/capsules/wg_scrollcapsule.h
	source/widgets/capsules/wg_selectcapsule.cpp
	source/widgets/capsules/wg_selectcapsule.h
	source/widgets/capsules/wg_sizecapsule.cpp
	source/widgets/capsules/wg_sizecapsule.h

	source/widgets/overlays/wg_dragndropoverlay.cpp
	source/widgets/overlays/wg_dragndropoverlay.h
	source/widgets/overlays/wg_overlay.cpp
	source/widgets/overlays/wg_overlay.h
	source/widgets/overlays/wg_modaloverlay.cpp
	source/widgets/overlays/wg_modaloverlay.h
	source/widgets/overlays/wg_popupoverlay.cpp
	source/widgets/overlays/wg_popupoverlay.h
	source/widgets/overlays/wg_tooltipoverlay.cpp
	source/widgets/overlays/wg_tooltipoverlay.h

	source/widgets/panels/wg_drawerpanel.cpp
	source/widgets/panels/wg_drawerpanel.h
	source/widgets/panels/wg_flexpanel.cpp
	source/widgets/panels/wg_flexpanel.h
	source/widgets/panels/wg_lambdapanel.cpp
	source/widgets/panels/wg_lambdapanel.h
	source/widgets/panels/wg_nodepanel.cpp
	source/widgets/panels/wg_nodepanel.h
	source/widgets/panels/wg_packpanel.cpp
	source/widgets/panels/wg_packpanel.h
	source/widgets/panels/wg_panel.cpp
	source/widgets/panels/wg_panel.h
	source/widgets/panels/wg_panel.impl.h
	source/widgets/panels/wg_rootpanel.cpp
	source/widgets/panels/wg_rootpanel.h
	source/widgets/panels/wg_scrollpanel.cpp
	source/widgets/panels/wg_scrollpanel.h
	source/widgets/panels/wg_splitpanel.cpp
	source/widgets/panels/wg_splitpanel.h
	source/widgets/panels/wg_stackpanel.cpp
	source/widgets/panels/wg_stackpanel.h
	source/widgets/panels/wg_tablepanel.cpp
	source/widgets/panels/wg_tablepanel.h
	source/widgets/panels/wg_twoslotpanel.cpp
	source/widgets/panels/wg_twoslotpanel.h

	wondergui.h
)

# The oldskool widget kit is compiled into the library, since it embeds its
# skin blocks.

set(WIDGETKIT_SOURCES
	widgetkits/wg_oldskool.h
	widgetkits/wg_oldskool.cpp
	widgetkits/wg_oldskool_resources.cpp
)


set(INCDIRS
	${WAX_INCDIRS}
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/source/base/
	${CMAKE_CURRENT_LIST_DIR}/source/components/
	${CMAKE_CURRENT_LIST_DIR}/source/numberlayouts/
	${CMAKE_CURRENT_LIST_DIR}/source/skins/
	${CMAKE_CURRENT_LIST_DIR}/source/skins/anim/
	${CMAKE_CURRENT_LIST_DIR}/source/skins/meter/
	${CMAKE_CURRENT_LIST_DIR}/source/skins/misc/
	${CMAKE_CURRENT_LIST_DIR}/source/skins/state/
	${CMAKE_CURRENT_LIST_DIR}/source/skins/static/
	${CMAKE_CURRENT_LIST_DIR}/source/slots/
	${CMAKE_CURRENT_LIST_DIR}/source/textlayouts/
	${CMAKE_CURRENT_LIST_DIR}/source/widgets/
	${CMAKE_CURRENT_LIST_DIR}/source/widgets/capsules/
	${CMAKE_CURRENT_LIST_DIR}/source/widgets/lists/
	${CMAKE_CURRENT_LIST_DIR}/source/widgets/overlays/
	${CMAKE_CURRENT_LIST_DIR}/source/widgets/panels/
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
source_group(widgetkits FILES ${WIDGETKIT_SOURCES})

add_library( wondergui STATIC ${SOURCES} ${WIDGETKIT_SOURCES} )
generate_export_header(wondergui BASE_NAME wg)

target_include_directories(wondergui PRIVATE ${INCDIRS} 
	"${CMAKE_CURRENT_BINARY_DIR}"  # For generated header
)

target_link_libraries(wondergui PUBLIC wax )

list(APPEND SOURCES ${WIDGETKIT_SOURCES})
list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WONDERGUI_SOURCES ${SOURCES} PARENT_SCOPE)
set(WONDERGUI_INCDIRS ${INCDIRS} PARENT_SCOPE)
