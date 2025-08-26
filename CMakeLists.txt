cmake_minimum_required(VERSION 3.5)

project(It VERSION 3.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS -H)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
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
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
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
        README.md
        paramsmodel.h paramsmodel.cpp
        filesmodel.h filesmodel.cpp
        colormappreview.h colormappreview.cpp
        compile_macos.sh
        syntaxhighlightercpp.h syntaxhighlightercpp.cpp
        ${app_icon_macos}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET It APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(It SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(It
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Icon
if(APPLE)
    set_target_properties(It PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
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

target_link_libraries(It PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.it)
endif()
set_target_properties(It PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS It
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(It)
endif()
