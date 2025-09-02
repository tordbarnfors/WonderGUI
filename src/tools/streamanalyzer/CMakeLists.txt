set(CMAKE_CXX_STANDARD 20)

set( SOURCES

	streamanalyzer.cpp
	streamanalyzer.h
)

set(RESOURCES
	${WG_ROOT}/resources/greyskin/plate.bmp
	${WG_ROOT}/resources/greyskin/button.bmp
	${WG_ROOT}/resources/greyskin/state_button.bmp
	${WG_ROOT}/resources/greyskin/checkbox.png
	${WG_ROOT}/resources/DroidSans.ttf
)

set(FRAMEWORKS
    ${WG_ROOT}/SDL2.framework
    ${WG_ROOT}/SDL2_image.framework
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(streamanalyzer MACOSX_BUNDLE
	${SOURCES}
	${RESOURCES}
)

target_include_directories(streamanalyzer PRIVATE 
	${WAPP_INCDIRS}
	${WGGFXSTREAM_INCDIRS}
	${WGSOFTGFX_INCDIRS}
	${WGDEBUGGER_INCDIRS}
)

target_working_directory(streamanalyzer ${WG_ROOT} )

target_link_libraries(streamanalyzer PRIVATE
	${WAPPLIB}
	wggfxstream
	wgsoftgfx
	wgdebugger
	wg_theme_simplistic
)

SET_SOURCE_FILES_PROPERTIES(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set_target_properties(streamanalyzer PROPERTIES

    XCODE_ATTRIBUTE_PRODUCT_NAME "streamanalyzer"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.wondergui.streamanalyzer"

    XCODE_ATTRIBUTE_MARKETING_VERSION "1.0.0"
    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "1.0.0"
    XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE YES
    XCODE_ATTRIBUTE_INFOPLIST_FILE  ""
    
    XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleDisplayName "WonderGUI StreamAnalyzer"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_LSApplicationCategoryType "public.app-category.uilities"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_NSPrincipalClass  "NSApplication"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_NSMainStoryboardFile "Main"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_NSHumanReadableCopyright "(c) 2024 Nordic UI Technologies AB"
    
    XCODE_EMBED_FRAMEWORKS          "${FRAMEWORKS}"
    XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY FALSE
    XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY TRUE
 
    INSTALL_RPATH @executable_path/../Frameworks
)


