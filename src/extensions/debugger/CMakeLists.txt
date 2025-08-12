set(SOURCES
    wg_idebugger.h
    wg_debugger.h
    wg_debugger.cpp
    wg_debugpanel.h
    wg_debugpanel.cpp
    wg_debugwindow.h
    wg_debugwindow.cpp
    
    wg_debugoverlay.h
    wg_debugoverlay.cpp

    wg_debugfrontend.h
    wg_debugfrontend.cpp
    wg_debugcapsule.h
    wg_debugcapsule.cpp

    wg_dummyinfopanel.h
    wg_dummyinfopanel.cpp

    windows/wg_msglogviewer.h
    windows/wg_msglogviewer.cpp
    windows/wg_objectinspector.h
    windows/wg_objectinspector.cpp
    windows/wg_objectinspector.h
    windows/wg_objectinspector.cpp
    windows/wg_objectlist.h
    windows/wg_objectlist.cpp
    windows/wg_skininspector.h
    windows/wg_skininspector.cpp
    windows/wg_textstyleinspector.h
    windows/wg_textstyleinspector.cpp
    windows/wg_widgettreeview.h
    windows/wg_widgettreeview.cpp

    objectinfopanels/wg_buttoninfopanel.h
    objectinfopanels/wg_buttoninfopanel.cpp
    objectinfopanels/wg_capsuleinfopanel.h
    objectinfopanels/wg_capsuleinfopanel.cpp
    objectinfopanels/wg_sizecapsuleinfopanel.h
    objectinfopanels/wg_sizecapsuleinfopanel.cpp
    objectinfopanels/wg_selectcapsuleinfopanel.h
    objectinfopanels/wg_selectcapsuleinfopanel.cpp
    objectinfopanels/wg_reordercapsuleinfopanel.h
    objectinfopanels/wg_reordercapsuleinfopanel.cpp
    objectinfopanels/wg_containerinfopanel.h
    objectinfopanels/wg_containerinfopanel.cpp
    objectinfopanels/wg_fillerinfopanel.h
    objectinfopanels/wg_fillerinfopanel.cpp
    objectinfopanels/wg_flexpanelinfopanel.h
    objectinfopanels/wg_flexpanelinfopanel.cpp
    objectinfopanels/wg_objectinfopanel.h
    objectinfopanels/wg_objectinfopanel.cpp
    objectinfopanels/wg_packpanelinfopanel.h
    objectinfopanels/wg_packpanelinfopanel.cpp
    objectinfopanels/wg_panelinfopanel.h
    objectinfopanels/wg_panelinfopanel.cpp
    objectinfopanels/wg_scrollpanelinfopanel.h
    objectinfopanels/wg_scrollpanelinfopanel.cpp
    objectinfopanels/wg_skininfopanel.h
    objectinfopanels/wg_skininfopanel.cpp
    objectinfopanels/wg_textdisplayinfopanel.h
    objectinfopanels/wg_textdisplayinfopanel.cpp
    objectinfopanels/wg_texteditorinfopanel.h
    objectinfopanels/wg_texteditorinfopanel.cpp
    objectinfopanels/wg_twoslotpanelinfopanel.h
    objectinfopanels/wg_twoslotpanelinfopanel.cpp
    objectinfopanels/wg_widgetinfopanel.h
    objectinfopanels/wg_widgetinfopanel.cpp

    slotinfopanels/wg_staticslotinfopanel.h
    slotinfopanels/wg_staticslotinfopanel.cpp
    slotinfopanels/wg_panelslotinfopanel.h
    slotinfopanels/wg_panelslotinfopanel.cpp
    slotinfopanels/wg_packpanelslotinfopanel.h
    slotinfopanels/wg_packpanelslotinfopanel.cpp
    slotinfopanels/wg_flexpanelslotinfopanel.h
    slotinfopanels/wg_flexpanelslotinfopanel.cpp
    slotinfopanels/wg_twoslotpanelslotinfopanel.h
    slotinfopanels/wg_twoslotpanelslotinfopanel.cpp

    componentinfopanels/wg_statictextinfopanel.h
    componentinfopanels/wg_statictextinfopanel.cpp
    componentinfopanels/wg_editabletextinfopanel.h
    componentinfopanels/wg_editabletextinfopanel.cpp
    componentinfopanels/wg_iconinfopanel.h
    componentinfopanels/wg_iconinfopanel.cpp
    componentinfopanels/wg_scrollerinfopanel.h
    componentinfopanels/wg_scrollerinfopanel.cpp
)

set(INCDIRS
   ${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wgdebugger STATIC ${SOURCES} )

target_include_directories(wgdebugger PRIVATE
    ${INCDIRS}
    ${WONDERGUI_INCDIRS}
)

set_target_properties( wgdebugger PROPERTIES FOLDER "Extensions" )


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGDEBUGGER_SOURCES ${SOURCES} PARENT_SCOPE)
set(WGDEBUGGER_INCDIRS ${INCDIRS} PARENT_SCOPE)