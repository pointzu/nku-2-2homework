cmake_minimum_required(VERSION 3.16)

project(algaeplus VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 新增：查找 Qt 多媒体模块（解决 QSoundEffect/QMediaPlayer 依赖）
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    # 明确列出所有自定义源文件（根据你的项目结构补充）
    algaetype.h algaetype.cpp
    algaecell.h algaecell.cpp
    gamegrid.h gamegrid.cpp
    gameresources.h gameresources.cpp
    algaegame.h algaegame.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(algaeplus
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(algaeplus SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(algaeplus
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# 新增：链接多媒体模块（解决 QSoundEffect 未定义引用错误）
target_link_libraries(algaeplus PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)

# 平台相关配置（保持原有逻辑）
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
