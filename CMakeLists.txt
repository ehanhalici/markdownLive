cmake_minimum_required(VERSION 3.16)

project(markdownLive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) # Q_OBJECT makrosu olan dosyaları otomatik işle
set(CMAKE_AUTORCC ON) # Kaynak (.qrc) dosyalarını otomatik işle
set(CMAKE_AUTOUIC ON) # .ui dosyalarını otomatik işle

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MarkdownRules.cpp
    src/SetupUi.cpp
    src/eventFilter.cpp
    src/CodeEditor.cpp
    include/MainWindow.hpp
    include/CodeEditor.hpp
)

add_executable(markdownLive ${PROJECT_SOURCES})

target_include_directories(markdownLive PRIVATE include)

target_link_libraries(markdownLive PRIVATE Qt6::Widgets)
