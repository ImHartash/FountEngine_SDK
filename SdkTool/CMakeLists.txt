cmake_minimum_required(VERSION 3.16)
project(sdk_tool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(sdk_tool
    src/main.cpp
    src/CAssetTreeView/CAssetTreeView.hpp
    src/CConverterRunner/CConverterRunner.hpp
    src/CConverterRunner/CConverterRunner.cpp
    src/MainWindow.hpp
    src/MainWindow.cpp
    src/CSettingsManager/CSettingsManager.hpp
    src/CSettingsDialog/CSettingsDialog.hpp
    src/CSettingsDialog/CSettingsDialog.cpp
    src/CFileIconProvider/CFileIconProvider.hpp
    src/CAssetFileSystemModel/CAssetFileSystemModel.hpp
    src/CFntFormats.hpp
    src/CPackExplorerDialog/CPackExplorerDialog.hpp
    src/CPackExplorerDialog/CPackExplorerDialog.cpp
    src/CMaterialCreatorDialog/CMaterialCreatorDialog.hpp
    src/CMaterialCreatorDialog/CMaterialCreatorDialog.cpp
)
target_link_libraries(sdk_tool PRIVATE Qt6::Widgets)

if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(QT_DLL_SUFFIX "d")
        set(QT_PLUGIN_SUFFIX "d")
    else()
        set(QT_DLL_SUFFIX "")
        set(QT_PLUGIN_SUFFIX "")
    endif()
    
    set(QT_DLLS
        "${CMAKE_PREFIX_PATH}/bin/Qt6Core${QT_DLL_SUFFIX}.dll"
        "${CMAKE_PREFIX_PATH}/bin/Qt6Widgets${QT_DLL_SUFFIX}.dll"
        "${CMAKE_PREFIX_PATH}/bin/Qt6Gui${QT_DLL_SUFFIX}.dll"
    )
    
    add_custom_command(TARGET sdk_tool POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${QT_DLLS}
            $<TARGET_FILE_DIR:sdk_tool>
        COMMENT "Copying Qt DLLs..."
    )
    
    add_custom_command(TARGET sdk_tool POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_PREFIX_PATH}/plugins/platforms"
            "$<TARGET_FILE_DIR:sdk_tool>/plugins/platforms"
        COMMENT "Copying Qt platform plugins..."
    )
endif()