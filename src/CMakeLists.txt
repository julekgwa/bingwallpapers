cmake_minimum_required(VERSION 3.21)

project(bingwallpapers VERSION 2026.11 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network Qml Quick QuickControls2 QuickDialogs2 Sql)
find_package(yaml-cpp REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_AUTORCC ON)

qt_add_executable(bingwallpapers
    main.cpp
    bingfeed.cpp
    bingfeed.h
    bingio.cpp
    bingio.h
    commandlineparser.cpp
    commandlineparser.h
    dbmanager.cpp
    dbmanager.h
    wallpapersetter.cpp
    wallpapersetter.h
    images.qrc
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(bingwallpapers
    URI BingApp
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/Icon.qml
        qml/AppSwitch.qml
        qml/ActionButton.qml
        qml/FolderField.qml
        qml/PagePane.qml
        qml/RegionComboBox.qml
        qml/SectionCard.qml
        qml/Segmented.qml
        qml/Stepper.qml
        qml/ToggleRow.qml
        qml/WallpaperPage.qml
        qml/SchedulePage.qml
        qml/StoragePage.qml
)

target_compile_definitions(bingwallpapers PRIVATE
    BINGWALLPAPERS_VERSION="${PROJECT_VERSION}"
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(bingwallpapers PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Quick
    Qt6::QuickControls2 Qt6::Sql
    yaml-cpp::yaml-cpp
)

include(GNUInstallDirs)
install(TARGETS bingwallpapers RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES bingwallpapers.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES bingwallpapers.png DESTINATION ${CMAKE_INSTALL_DATADIR}/bingwallpapers)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)

    # The tests link the non-UI sources directly.
    qt_add_executable(tst_bingwallpapers
        tests/tst_bingwallpapers.cpp
        bingfeed.cpp
        bingio.cpp
        commandlineparser.cpp
        dbmanager.cpp
        wallpapersetter.cpp
    )
    target_include_directories(tst_bingwallpapers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(tst_bingwallpapers PRIVATE BINGWALLPAPERS_VERSION="${PROJECT_VERSION}")
    target_link_libraries(tst_bingwallpapers PRIVATE
        Qt6::Core Qt6::Network Qt6::Qml Qt6::Sql Qt6::Test yaml-cpp::yaml-cpp
    )
    add_test(NAME tst_bingwallpapers COMMAND tst_bingwallpapers)
endif()
