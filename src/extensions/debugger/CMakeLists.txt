set(SOURCES
    wg_debugger.h

    core/wg_debugbackend.h
    core/wg_debugbackend.cpp
    core/wg_debugtheme.h
    core/wg_idebugcontext.h
    core/wg_infosection.h
    core/wg_infosection.cpp
    core/wg_typedinfosection.h
    core/wg_inspectorview.h
    core/wg_inspectorview.cpp

    sections/wg_dummyinfosection.h
    sections/wg_dummyinfosection.cpp

    sections/objects/wg_buttoninfosection.h
    sections/objects/wg_buttoninfosection.cpp
    sections/objects/wg_canvascapsuleinfosection.h
    sections/objects/wg_canvascapsuleinfosection.cpp
    sections/objects/wg_capsuleinfosection.h
    sections/objects/wg_capsuleinfosection.cpp
    sections/objects/wg_containerinfosection.h
    sections/objects/wg_containerinfosection.cpp
    sections/objects/wg_fillerinfosection.h
    sections/objects/wg_fillerinfosection.cpp
    sections/objects/wg_flexpanelinfosection.h
    sections/objects/wg_flexpanelinfosection.cpp
    sections/objects/wg_labelcapsuleinfosection.h
    sections/objects/wg_labelcapsuleinfosection.cpp
    sections/objects/wg_objectinfosection.h
    sections/objects/wg_objectinfosection.cpp
    sections/objects/wg_packpanelinfosection.h
    sections/objects/wg_packpanelinfosection.cpp
    sections/objects/wg_paddingcapsuleinfosection.h
    sections/objects/wg_paddingcapsuleinfosection.cpp
    sections/objects/wg_panelinfosection.h
    sections/objects/wg_panelinfosection.cpp
    sections/objects/wg_renderlayercapsuleinfosection.h
    sections/objects/wg_renderlayercapsuleinfosection.cpp
    sections/objects/wg_reordercapsuleinfosection.h
    sections/objects/wg_reordercapsuleinfosection.cpp
    sections/objects/wg_scalecapsuleinfosection.h
    sections/objects/wg_scalecapsuleinfosection.cpp
    sections/objects/wg_scrollpanelinfosection.h
    sections/objects/wg_scrollpanelinfosection.cpp
    sections/objects/wg_selectcapsuleinfosection.h
    sections/objects/wg_selectcapsuleinfosection.cpp
    sections/objects/wg_sizecapsuleinfosection.h
    sections/objects/wg_sizecapsuleinfosection.cpp
    sections/objects/wg_skininfosection.h
    sections/objects/wg_skininfosection.cpp
    sections/objects/wg_textdisplayinfosection.h
    sections/objects/wg_textdisplayinfosection.cpp
    sections/objects/wg_texteditorinfosection.h
    sections/objects/wg_texteditorinfosection.cpp
    sections/objects/wg_twoslotpanelinfosection.h
    sections/objects/wg_twoslotpanelinfosection.cpp
    sections/objects/wg_widgetinfosection.h
    sections/objects/wg_widgetinfosection.cpp

    sections/slots/wg_flexpanelslotinfosection.h
    sections/slots/wg_flexpanelslotinfosection.cpp
    sections/slots/wg_packpanelslotinfosection.h
    sections/slots/wg_packpanelslotinfosection.cpp
    sections/slots/wg_panelslotinfosection.h
    sections/slots/wg_panelslotinfosection.cpp
    sections/slots/wg_staticslotinfosection.h
    sections/slots/wg_staticslotinfosection.cpp
    sections/slots/wg_twoslotpanelslotinfosection.h
    sections/slots/wg_twoslotpanelslotinfosection.cpp

    sections/components/wg_editabletextinfosection.h
    sections/components/wg_editabletextinfosection.cpp
    sections/components/wg_glowinfosection.h
    sections/components/wg_glowinfosection.cpp
    sections/components/wg_iconinfosection.h
    sections/components/wg_iconinfosection.cpp
    sections/components/wg_scrollerinfosection.h
    sections/components/wg_scrollerinfosection.cpp
    sections/components/wg_statictextinfosection.h
    sections/components/wg_statictextinfosection.cpp

    views/wg_msglogviewer.h
    views/wg_msglogviewer.cpp
    views/wg_objectinspector.h
    views/wg_objectinspector.cpp
    views/wg_skininspector.h
    views/wg_skininspector.cpp
    views/wg_textstyleinspector.h
    views/wg_textstyleinspector.cpp
    views/wg_widgetinspector.h
    views/wg_widgetinspector.cpp
    views/wg_widgettreeview.h
    views/wg_widgettreeview.cpp

    frontend/wg_debugcapsule.h
    frontend/wg_debugcapsule.cpp
    frontend/wg_debugfrontend.h
    frontend/wg_debugfrontend.cpp
    frontend/wg_debugwindow.h
    frontend/wg_debugwindow.cpp

    overlay/wg_debugoverlay.h
    overlay/wg_debugoverlay.cpp
)

# Subdirectories come before the root so that stale headers left behind in the
# old flat layout can never shadow the moved ones.

set(INCDIRS
    ${CMAKE_CURRENT_LIST_DIR}/core
    ${CMAKE_CURRENT_LIST_DIR}/sections
    ${CMAKE_CURRENT_LIST_DIR}/sections/objects
    ${CMAKE_CURRENT_LIST_DIR}/sections/slots
    ${CMAKE_CURRENT_LIST_DIR}/sections/components
    ${CMAKE_CURRENT_LIST_DIR}/views
    ${CMAKE_CURRENT_LIST_DIR}/frontend
    ${CMAKE_CURRENT_LIST_DIR}/overlay
    ${CMAKE_CURRENT_LIST_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library( wgdebugger STATIC ${SOURCES} )

target_include_directories(wgdebugger PRIVATE
    ${INCDIRS}
    ${WONDERGUI_INCDIRS}
	${WG_ROOT}/src/
)

set_target_properties( wgdebugger PROPERTIES FOLDER "Extensions" )


list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/ )
set(WGDEBUGGER_SOURCES ${SOURCES} PARENT_SCOPE)
set(WGDEBUGGER_INCDIRS ${INCDIRS} PARENT_SCOPE)