cmake_minimum_required(VERSION 3.16)
project(CyberScanner VERSION 1.0.0 LANGUAGES CXX)

# Qt configuration
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# Source files
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resources.qrc
)

# Create executable
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CyberScanner
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(CyberScanner ${PROJECT_SOURCES})
endif()

# Link libraries
target_link_libraries(CyberScanner PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

# Platform-specific properties
if(WIN32)
    # Windows-specific settings
    set_target_properties(CyberScanner PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "CyberScanner"
    )

    # Add Windows icon resource if available
    if(EXISTS "${CMAKE_SOURCE_DIR}/app.rc")
        target_sources(CyberScanner PRIVATE app.rc)
    endif()

elseif(APPLE)
    # macOS-specific settings
    set_target_properties(CyberScanner PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.cybernilsen.CyberScanner"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_BUNDLE_NAME "CyberScanner"
        MACOSX_BUNDLE_INFO_STRING "Fast Port Scanner v${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "© 2025 CyberNilsen"
    )

    # Add app icon for macOS
    if(EXISTS "${CMAKE_SOURCE_DIR}/app.icns")
        set_target_properties(CyberScanner PROPERTIES
            MACOSX_BUNDLE_ICON_FILE "app.icns"
        )
        set_source_files_properties("${CMAKE_SOURCE_DIR}/app.icns" PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
        target_sources(CyberScanner PRIVATE "${CMAKE_SOURCE_DIR}/app.icns")
    endif()

else()
    # Linux-specific settings
    set_target_properties(CyberScanner PROPERTIES
        OUTPUT_NAME "cyberscanner"
    )
endif()

# Installation rules
include(GNUInstallDirs)

if(WIN32)
    install(TARGETS CyberScanner
        RUNTIME DESTINATION .
    )
elseif(APPLE)
    install(TARGETS CyberScanner
        BUNDLE DESTINATION .
    )
else()
    install(TARGETS CyberScanner
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Install desktop file for Linux
    if(EXISTS "${CMAKE_SOURCE_DIR}/cyberscanner.desktop")
        install(FILES cyberscanner.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
        )
    endif()
endif()

# Qt finalization
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CyberScanner)
endif()
