set(SOURCES
    wg_debugger.h
    wg_debugger.cpp
    wg_debugpanel.h
    wg_debugpanel.cpp
    wg_debugoverlay.h
    wg_debugoverlay.cpp

    wg_dummyinfopanel.h
    wg_dummyinfopanel.cpp
    wg_msglogpanel.h
    wg_msglogPanel.cpp
    wg_widgettreepanel.h
    wg_widgettreepanel.cpp

    objectinfopanels/wg_containerinfopanel.h
    objectinfopanels/wg_containerinfopanel.cpp
    objectinfopanels/wg_objectinfopanel.h
    objectinfopanels/wg_objectinfopanel.cpp
    objectinfopanels/wg_panelinfopanel.h
    objectinfopanels/wg_panelinfopanel.cpp
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