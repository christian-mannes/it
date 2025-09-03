cmake_minimum_required(VERSION 3.5)

project(It VERSION 3.0.1 LANGUAGES CXX)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64") # Fat binaries
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0") # Minimum macOS version
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.mannestech.it3")
    set(CODE_SIGN_IDENTITY "Developer ID Application: Christian Mannes (6CL5P9A99V)")
    set(DEVELOPMENT_TEAM "6CL5P9A99V")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
#set(QT_DEBUG_FIND_PACKAGE ON)
#set(CMAKE_CXX_FLAGS -H)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPP_VERSION=\"${PROJECT_VERSION}\"")

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport Svg)
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/it
)

qt_add_executable(It
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    resources.qrc
    itview.h itview.cpp
    it/Args.h
    it/Args.cpp
    it/MTComplex.h it/MTComplex.cpp
    it/MTRandom.cpp it/MTRandom.h
    it/Function.h it/Function.cpp
    it/State.cpp it/State.h
    it/Colormap.h it/Colormap.cpp
    it/Algo.h it/Algo.cpp
    it/FUN.cpp
    TODO.md
    paramsmodel.h paramsmodel.cpp

    colormappreview.h colormappreview.cpp
    compile_macos.sh
    syntaxhighlightercpp.h syntaxhighlightercpp.cpp
    it/debug.h it/debug.cpp
    ${app_icon_macos}
    jupyter.h jupyter.cpp
    template.txt
    tree.h tree.cpp
    README.md

)

file(GLOB_RECURSE IT_SOURCES "${CMAKE_SOURCE_DIR}/it/*")
file(GLOB_RECURSE MAP_FILES "${CMAKE_SOURCE_DIR}/maps/*")
set(IT_ZIP ${CMAKE_CURRENT_BINARY_DIR}/it.zip)
set(MAPS_ZIP ${CMAKE_CURRENT_BINARY_DIR}/maps.zip)

add_custom_command(
    OUTPUT ${IT_ZIP}
    COMMAND ${CMAKE_COMMAND} -E tar "cvf" ${IT_ZIP} --format=zip it
    DEPENDS ${IT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Creating zip.zip from it directory"
    VERBATIM
)
add_custom_command(
    OUTPUT ${MAPS_ZIP}
    COMMAND ${CMAKE_COMMAND} -E tar "cvf" ${MAPS_ZIP} --format=zip maps
    DEPENDS ${MAP_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Creating maps.zip from maps directory"
    VERBATIM
)

if(APPLE)
    target_sources(It PRIVATE compile_macos.sh)
    set_source_files_properties(compile_macos.sh PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(It PRIVATE template.txt)
    set_source_files_properties(template.txt PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(It PRIVATE ${IT_ZIP})
    set_source_files_properties(${IT_ZIP} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(It PRIVATE ${MAPS_ZIP})
    set_source_files_properties(${MAPS_ZIP} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)


    set_target_properties(It PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
        OSX_ARCHITECTURES "arm64;x86_64"
        MACOSX_BUNDLE_BUNDLE_NAME "It"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_IDENTIFIER ${MACOSX_BUNDLE_GUI_IDENTIFIER}
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODE_SIGN_IDENTITY}
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM}
        XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS YES
    )
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/icon.icns"
        "${CMAKE_CURRENT_BINARY_DIR}/icon.icns"
        COPYONLY
    )
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/icon.icns"
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
    target_sources(It PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/icon.icns")
endif()

target_link_libraries(It PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::PrintSupport Qt6::Svg Qt6::WebEngineWidgets)
target_link_libraries(It PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(TARGETS It
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(It)
endif()
