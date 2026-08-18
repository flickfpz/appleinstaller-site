cmake_minimum_required(VERSION 3.16)
project(AppleInstaller VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Concurrent)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/AppCard.cpp
    src/CategoryPanel.cpp
    src/InstallWorker.cpp
    src/ProgressOverlay.cpp
    src/Theme.cpp
    src/UpdateChecker.cpp
)

set(HEADERS
    src/MainWindow.h
    src/AppCard.h
    src/AppData.h
    src/CategoryPanel.h
    src/InstallWorker.h
    src/ProgressOverlay.h
    src/Theme.h
    src/OsDetect.h
    src/UpdateChecker.h
)

qt6_add_resources(RESOURCES resources/resources.qrc)

add_executable(AppleInstaller
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(AppleInstaller PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

target_include_directories(AppleInstaller PRIVATE src)

# ── Platform tweaks ───────────────────────────────────────────────────────────
if(WIN32)
    # Hide the console window on Windows
    set_target_properties(AppleInstaller PROPERTIES WIN32_EXECUTABLE TRUE)

    # Embed a basic manifest so winget UAC prompts work correctly
    if(MSVC)
        target_sources(AppleInstaller PRIVATE src/app.manifest)
    endif()

elseif(APPLE)
    set_target_properties(AppleInstaller PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME        "App Installer"
        MACOSX_BUNDLE_GUI_IDENTIFIER     "com.kiro.appinstaller"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}"
    )
endif()

# ── Installation rules (used by `cmake --install`) ───────────────────────────
include(GNUInstallDirs)

if(APPLE)
    install(TARGETS AppleInstaller
        BUNDLE  DESTINATION /Applications
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
elseif(UNIX)
    install(TARGETS AppleInstaller
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # .desktop file
    install(FILES "${CMAKE_SOURCE_DIR}/resources/appinstaller.desktop"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
        OPTIONAL
    )

    # Icon
    install(FILES "${CMAKE_SOURCE_DIR}/resources/icons/appinstaller.png"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps"
        OPTIONAL
    )
elseif(WIN32)
    install(TARGETS AppleInstaller
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    # On Windows, use windeployqt as a post-install step
    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT)
        install(CODE "
            execute_process(
                COMMAND \"${WINDEPLOYQT}\" --release --no-translations
                        \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/AppleInstaller.exe\"
            )
        ")
    endif()
endif()

# ── CPack packaging ───────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME              "AppInstaller")
set(CPACK_PACKAGE_VENDOR            "Kiro")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Install your favourite apps in one click")
set(CPACK_PACKAGE_VERSION           "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AppInstaller")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_SOURCE_DIR}/LICENSE.txt")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME     "App Installer")
    set(CPACK_NSIS_PACKAGE_NAME     "AppInstaller")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortcut '$SMPROGRAMS\\\\App Installer.lnk' '$INSTDIR\\\\bin\\\\AppleInstaller.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\App Installer.lnk'")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "AppInstaller")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")

    # DEB
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kiro <hello@kiro.dev>")
    set(CPACK_DEBIAN_PACKAGE_SECTION    "utils")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libqt6widgets6, libqt6network6, libqt6concurrent6")

    # RPM
    set(CPACK_RPM_PACKAGE_LICENSE       "MIT")
    set(CPACK_RPM_PACKAGE_GROUP         "Applications/System")
    set(CPACK_RPM_PACKAGE_REQUIRES      "qt6-qtbase >= 6.2")
endif()

include(CPack)

# ── Summary ───────────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "  ┌─ AppleInstaller build configuration ────────────")
message(STATUS "  │  Version   : ${PROJECT_VERSION}")
message(STATUS "  │  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  │  Qt version: ${Qt6_VERSION}")
message(STATUS "  │  Install to: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
message(STATUS "  └──────────────────────────────────────────────────")
message(STATUS "")
