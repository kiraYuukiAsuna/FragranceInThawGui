cmake_minimum_required(VERSION 3.5)

project(FragranceInThawGui LANGUAGES CXX)

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message("Debug mode:${CMAKE_C_FLAGS_DEBUG}")

elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    message("Release mode:${CMAKE_C_FLAGS_RELEASE}")

else()
    message("else:${CMAKE_BUILD_TYPE}")
    message("else:${CMAKE_C_FLAGS_RELEASE}")
    message("else:Attention!!!will be treated as release version...")

endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

include_directories(
    "Thirdparty/OpenCV4.5.5/include"
    "Thirdparty/OpenCV4.5.5/include/opencv2"
    "Thirdparty/onnxruntime-win-x64-1.10.0/include"
    "Thirdparty/json"
    )

link_directories(
    "Thirdparty/OpenCV4.5.5/x64/vc17/lib"
    "Thirdparty/onnxruntime-win-x64-1.10.0/lib"
    )

set(UI_SOURCES
    src/mainwindow.ui
    src/welcomewindow.ui
    src/developmodewindow.ui
    src/aboutdialog.ui
    src/helpdialog.ui
    )

set(HEADERS
    src/mainwindow.h
    src/welcomewindow.h
    src/developmodewindow.h
    src/aboutdialog.h
    src/helpdialog.h

    src/ConstantDefination.h
    src/videoprocessing.h

    src/Log/Log.h

    src/RVMInvoke/RVMInvoke.h

    src/configmanager.h

    )

set(SOURCES
    main.cpp
    src/mainwindow.cpp
    src/welcomewindow.cpp
    src/developmodewindow.cpp
    src/aboutdialog.cpp
    src/helpdialog.cpp

    src/videoprocessing.cpp

    src/Log/Log.cpp

    src/RVMInvoke/RVMInvoke.cpp

    src/configmanager.cpp

    )

add_executable(FragranceInThawGui
    ${UI_SOURCES}
    ${HEADERS}
    ${SOURCES}
    "app_win32.rc"
    )

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message("Debug mode:${CMAKE_C_FLAGS_DEBUG}")
    target_link_libraries(FragranceInThawGui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        opencv_world455d.lib
        onnxruntime.lib
        )

elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    message("Release mode:${CMAKE_C_FLAGS_RELEASE}")
    target_link_libraries(FragranceInThawGui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        opencv_world455.lib
        onnxruntime.lib
        )

else()
    message("else:${CMAKE_BUILD_TYPE}")
    message("else:${CMAKE_C_FLAGS_RELEASE}")
    message("else:Attention!!!will be treated as release version...")
    target_link_libraries(FragranceInThawGui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        opencv_world455d.lib
        onnxruntime.lib
        )
endif()

Set_Target_Properties(FragranceInThawGui PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
