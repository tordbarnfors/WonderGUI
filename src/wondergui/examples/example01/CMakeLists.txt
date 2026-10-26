set(CMAKE_CXX_STANDARD 20)



set(SOURCES
	example01.cpp
)

set(RESOURCES
	${WG_ROOT}/resources/simple_button.bmp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(example01 MACOSX_BUNDLE
	${SOURCES}
	${RESOURCES}
)

target_include_directories(example01 PRIVATE

	${SDL2_INCDIR}
)


target_link_directories(example01 PRIVATE 
	${SDL2_LIBDIR}
)

target_include_directories(example01 PRIVATE 
	${WONDERGUI_INCDIRS}
	${WAXSOFTGFX_INCDIRS}
)


target_working_directory(example01 ${WG_ROOT} )

target_link_libraries(example01 PUBLIC
	waxsoftgfx
	wondergui 
	${SDL2_LIBS}
)

SET_SOURCE_FILES_PROPERTIES(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set_target_properties(example01 PROPERTIES

    XCODE_ATTRIBUTE_PRODUCT_NAME "example01"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.wondergui.example01"

    XCODE_ATTRIBUTE_MARKETING_VERSION "1.0.0"
    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "1.0.0"
    XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE YES
    XCODE_ATTRIBUTE_INFOPLIST_FILE  ""
    
    XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleDisplayName "WonderGUI Example 1"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_LSApplicationCategoryType "public.app-category.games"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_NSPrincipalClass  "NSApplication"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_NSMainStoryboardFile "Main"
    XCODE_ATTRIBUTE_INFOPLIST_KEY_NSHumanReadableCopyright "(c) 2024 Nordic UI Technologies AB"
    
    XCODE_EMBED_FRAMEWORKS          ${WG_ROOT}/SDL2.framework
    XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY FALSE
    XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY TRUE
 
    INSTALL_RPATH @executable_path/../Frameworks
)
