cmake_minimum_required(VERSION 3.5)

project(It VERSION 4.0.3 LANGUAGES CXX)
if (WIN32)
    set(CPACK_PACKAGE_NAME "It")
    set(CPACK_PACKAGE_VERSION "4.0.3")
    set(CPACK_GENERATOR "NSIS")
endif()

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPP_VERSION=\"${PROJECT_VERSION}\" -D_CRT_SECURE_NO_WARNINGS" )

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport Svg Core Network)
#find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)

option(BUILD_QT_INSTALLER "Build installer using Qt Installer Framework" OFF)
# cmake -B build -DBUILD_QT_INSTALLER=ON
# cmake --build build --target qt_installer

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
    LICENSE.md
    deploy.sh
    doc/download.md doc/manual.md
    builtin_mandi.txt
    compile_windows.bat
    app-icon.rc
    compile_linux.sh
    deploy.bat
)

file(GLOB_RECURSE IT_SOURCES "${CMAKE_SOURCE_DIR}/it/*")
file(GLOB_RECURSE MAP_FILES "${CMAKE_SOURCE_DIR}/maps/*")
set(IT_ZIP ${CMAKE_CURRENT_BINARY_DIR}/it.zip)
set(MAPS_ZIP ${CMAKE_CURRENT_BINARY_DIR}/maps.zip)

if(APPLE)
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

    target_sources(It PRIVATE compile_macos.sh)
    set_source_files_properties(compile_macos.sh PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(It PRIVATE template.txt)
    set_source_files_properties(template.txt PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(It PRIVATE builtin_mandi.txt)
    set_source_files_properties(builtin_mandi.txt PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
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
    install(TARGETS It
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
elseif(WIN32)
    set_target_properties(It PROPERTIES WIN32_EXECUTABLE TRUE)
    target_sources(It PRIVATE app-icon.rc)
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar "cvf" ${IT_ZIP} --format=zip it
        DEPENDS ${IT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Creating zip.zip from it directory"
        VERBATIM
    )
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar "cvf" ${MAPS_ZIP} --format=zip maps
        DEPENDS ${MAP_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Creating maps.zip from maps directory"
        VERBATIM
    )
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/template.txt" "${CMAKE_CURRENT_BINARY_DIR}"
    )
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/builtin_mandi.txt" "${CMAKE_CURRENT_BINARY_DIR}"
    )
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compile_windows.bat" "${CMAKE_CURRENT_BINARY_DIR}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compile_windows.bat"
    )
    install(TARGETS It
        RUNTIME DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(TARGETS It RUNTIME DESTINATION .)
    install(FILES ${IT_ZIP} DESTINATION .)
    install(FILES ${MAPS_ZIP} DESTINATION .)
    install(FILES compile_windows.bat DESTINATION .)
    install(FILES template.txt DESTINATION .)
    install(FILES builtin_mandi.txt DESTINATION .)
    include(CPack)
else()
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar "cvf" ${IT_ZIP} --format=zip it
        DEPENDS ${IT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Creating zip.zip from it directory"
        VERBATIM
    )
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar "cvf" ${MAPS_ZIP} --format=zip maps
        DEPENDS ${MAP_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Creating maps.zip from maps directory"
        VERBATIM
    )
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/template.txt" "${CMAKE_CURRENT_BINARY_DIR}"
    )
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/builtin_mandi.txt" "${CMAKE_CURRENT_BINARY_DIR}"
    )
    add_custom_command(TARGET It POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compile_linux.sh" "${CMAKE_CURRENT_BINARY_DIR}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compile_linux.sh"
    )

    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/It)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")
    install(TARGETS It
        RUNTIME DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(FILES $<TARGET_RUNTIME_DLLS:It> DESTINATION lib)
    install(FILES ${CMAKE_SOURCE_DIR}/run.sh DESTINATION . PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

    install(FILES ${IT_ZIP} DESTINATION .)
    install(FILES ${MAPS_ZIP} DESTINATION .)
    install(FILES compile_linux.sh DESTINATION .)
    install(FILES template.txt DESTINATION .)
    install(FILES builtin_mandi.txt DESTINATION .)
endif()

target_link_libraries(It PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::PrintSupport Qt6::Svg Qt6::Core Qt6::Network)
#target_link_libraries(It PRIVATE Qt6::WebEngineWidgets)

include(GNUInstallDirs)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(It)
endif()



if(BUILD_QT_INSTALLER)
    # Find Qt Installer Framework
    find_program(QT_INSTALLER_BINARY
        NAMES binarycreator binarycreator.exe
        HINTS
            ${Qt6_DIR}/../../../Tools/QtInstallerFramework/*/bin
            "C:/Qt/Tools/QtInstallerFramework/*/bin"
        DOC "Qt Installer Framework binarycreator"
    )

    if(NOT QT_INSTALLER_BINARY)
        message(FATAL_ERROR "Qt Installer Framework not found! Install it via Qt Maintenance Tool.")
    endif()

    # Set installer directories
    set(INSTALLER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/installer")
    set(INSTALLER_BINARY_DIR "${CMAKE_BINARY_DIR}/installer")
    set(INSTALLER_DATA_DIR "${INSTALLER_SOURCE_DIR}/packages/com.mannes-tech.it/data")

    # Custom target to prepare installer data
    add_custom_target(prepare_qt_installer
        DEPENDS your_app_name

        # Clear and create data directory
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${INSTALLER_DATA_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALLER_DATA_DIR}"

        # Copy application executable
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:your_app_name>"
            "${INSTALLER_DATA_DIR}/"

        # Copy additional files (if any)
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resources"
            "${INSTALLER_DATA_DIR}/resources"

        # Copy license, readme, etc.
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/LICENSE.txt"
            "${INSTALLER_DATA_DIR}/"

        # Deploy Qt dependencies
        COMMAND ${WINDEPLOYQT_EXECUTABLE}
            "${INSTALLER_DATA_DIR}/$<TARGET_FILE_NAME:your_app_name>"
            --dir "${INSTALLER_DATA_DIR}"
            --no-translations  # Optional: exclude translations
            --no-system-d3d-compiler  # Optional: reduce size
            --no-opengl-sw  # Optional: reduce size

        # Copy installer config to build directory
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${INSTALLER_SOURCE_DIR}/config"
            "${INSTALLER_BINARY_DIR}/config"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${INSTALLER_SOURCE_DIR}/packages"
            "${INSTALLER_BINARY_DIR}/packages"

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Preparing Qt Installer data..."
    )

    # Create the installer
    add_custom_target(qt_installer
        DEPENDS prepare_qt_installer

        COMMAND "${QT_INSTALLER_BINARY}"
            --offline-only
            -c "${INSTALLER_BINARY_DIR}/config/config.xml"
            -p "${INSTALLER_BINARY_DIR}/packages"
            "${CMAKE_BINARY_DIR}/YourApp-Installer.exe"

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Creating Qt Installer executable..."
    )

    # Optional: Online installer
    add_custom_target(qt_installer_online
        DEPENDS prepare_qt_installer

        COMMAND "${QT_INSTALLER_BINARY}"
            --online-only
            -c "${INSTALLER_BINARY_DIR}/config/config.xml"
            -p "${INSTALLER_BINARY_DIR}/packages"
            "${CMAKE_BINARY_DIR}/YourApp-OnlineInstaller.exe"

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Creating Qt Online Installer..."
    )
endif()
