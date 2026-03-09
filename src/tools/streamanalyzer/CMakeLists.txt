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

	# These should be part of the WAP

	${WG_ROOT}/resources/NotoSans-Regular.ttf
	${WG_ROOT}/resources/NotoSans-Bold.ttf
	${WG_ROOT}/resources/NotoSans-Italic.ttf
	${WG_ROOT}/resources/DroidSansMono.ttf
	${WG_ROOT}/resources/skin_widgets.png

)

find_package(Freetype REQUIRED)

get_target_property(FREETYPE_DYLIB Freetype::Freetype IMPORTED_LOCATION)
if(NOT FREETYPE_DYLIB)
    get_target_property(FREETYPE_DYLIB Freetype::Freetype IMPORTED_LOCATION_RELEASE)
endif()


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
	Freetype::Freetype
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
 
#    INSTALL_RPATH @executable_path/../Frameworks
)

add_custom_command(TARGET streamanalyzer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_BUNDLE_DIR:streamanalyzer>/Contents/Frameworks"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${FREETYPE_DYLIB}"
        "$<TARGET_BUNDLE_DIR:streamanalyzer>/Contents/Frameworks/libfreetype.6.dylib"
    COMMAND install_name_tool -id "@rpath/libfreetype.6.dylib"
        "$<TARGET_BUNDLE_DIR:streamanalyzer>/Contents/Frameworks/libfreetype.6.dylib"
    COMMAND install_name_tool -change
        "/opt/homebrew/opt/freetype/lib/libfreetype.6.dylib"
        "@rpath/libfreetype.6.dylib"
        "$<TARGET_BUNDLE_DIR:streamanalyzer>/Contents/MacOS/streamanalyzer"
    COMMAND install_name_tool -add_rpath "@executable_path/../Frameworks"
        "$<TARGET_BUNDLE_DIR:streamanalyzer>/Contents/MacOS/streamanalyzer"
    COMMAND install_name_tool -delete_rpath "/opt/homebrew/opt/freetype/lib"
        "$<TARGET_BUNDLE_DIR:streamanalyzer>/Contents/MacOS/streamanalyzer"
    COMMAND install_name_tool -delete_rpath "/Library/Frameworks"
        "$<TARGET_BUNDLE_DIR:streamanalyzer>/Contents/MacOS/streamanalyzer"
    COMMAND codesign --force --deep --sign -
        "$<TARGET_BUNDLE_DIR:streamanalyzer>"
    COMMENT "Bundling libfreetype.6.dylib"
)
