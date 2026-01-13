cmake_minimum_required(VERSION 3.16)
project(TradingDashboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---

# 1. GLFW
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found, fetching...")
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
    )
    FetchContent_MakeAvailable(glfw)
endif()

# 2. Setup Include Directories
include_directories(
    libs/glad/include
    libs/imgui
    libs/imgui/backends
    libs/implot
    src  # Include src directory
)

# 3. Source Files
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui")
if(NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "imgui.cpp not found at ${IMGUI_DIR}/imgui.cpp. Please check your source tree.")
endif()

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(IMPLOT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/implot/implot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/implot/implot_items.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/implot/implot_demo.cpp
)

set(GLAD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/src/glad.c
)

set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/TradingEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/DashboardUI.cpp
)

# 4. Executable
add_executable(TradingDashboard
    src/main.cpp
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
    ${GLAD_SOURCES}
    ${APP_SOURCES}
)

# 5. Linking
if(glfw3_FOUND)
    target_link_libraries(TradingDashboard PRIVATE glfw)
else()
    target_link_libraries(TradingDashboard PRIVATE glfw)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(TradingDashboard PRIVATE pthread dl)
endif()