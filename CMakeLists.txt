cmake_minimum_required(VERSION 3.16)

project(algaeplus VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(algaeplus
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        algaetype.h algaetype.cpp
        algaecell.h algaecell.cpp
        gamegrid.h gamegrid.cpp
        gameresources.h gameresources.cpp
        algaegame.h algaegame.cpp
        image.qrc
        ../resources/background_music1.mp3.mp3 ../resources/background_music2.mp3.mp3 ../resources/planted.mp3 ../resources/victory.mp3
        ../resources/sounds/pause.wav
        ../resources/sounds/displant.wav
        ../resources/sounds/buzzer.wav
        ../resources/sounds/buzzer.wav
        SoundManager.h SoundManager.cpp
        ../resources/newbuzzer.wav ../resources/newdisplant.wav
        ../resources/buzzer_fixed.wav ../resources/displant_fixed.wav
        sound.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET algaeplus APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(algaeplus SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(algaeplus
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(algaeplus PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.algaeplus)
endif()
set_target_properties(algaeplus PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS algaeplus
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(algaeplus)
endif()
