cmake_minimum_required(VERSION 3.21)

project(OpenBMS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- SimpleBLE (cross-platform BLE library) ---
include(FetchContent)
FetchContent_Declare(
    simpleble
    GIT_REPOSITORY https://github.com/simpleble/simpleble.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  simpleble
)

FetchContent_MakeAvailable(simpleble)

# On Windows with MinGW, point SimpleBLE to the Windows SDK cppwinrt headers
# and force C++20 (WinRT coroutines require it)
if(WIN32 AND MINGW AND TARGET simpleble)
    file(GLOB _WIN_SDK_VERSIONS "C:/Program Files (x86)/Windows Kits/10/Include/10.*")
    list(SORT _WIN_SDK_VERSIONS ORDER DESCENDING)
    list(GET _WIN_SDK_VERSIONS 0 _WIN_SDK_INCLUDE)
    if(EXISTS "${_WIN_SDK_INCLUDE}/cppwinrt")
        target_include_directories(simpleble SYSTEM PRIVATE "${_WIN_SDK_INCLUDE}/cppwinrt")
        message(STATUS "Using WinRT headers from: ${_WIN_SDK_INCLUDE}/cppwinrt")
    else()
        message(WARNING "Windows SDK cppwinrt headers not found — BLE will not build")
    endif()
    set_target_properties(simpleble PROPERTIES CXX_STANDARD 20)
endif()

# --- Qt ---
find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appOpenBMS
    main.cpp
)

# Embed the application icon on Windows
if(WIN32)
    target_sources(appOpenBMS PRIVATE app.rc)
endif()

qt_add_qml_module(appOpenBMS
    URI OpenBMS
    QML_FILES
        Main.qml
        ScanPage.qml
        DashboardPage.qml
        CellBar.qml
        StatCard.qml
    SOURCES
        blemanager.h blemanager.cpp
        bmsdata.h
)

set_target_properties(appOpenBMS PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appOpenBMS
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appOpenBMS
    PRIVATE Qt6::Quick simpleble::simpleble
)

# WinRT runtime libraries needed by SimpleBLE on Windows
if(WIN32)
    target_link_libraries(appOpenBMS PRIVATE runtimeobject)
endif()

include(GNUInstallDirs)
install(TARGETS appOpenBMS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
