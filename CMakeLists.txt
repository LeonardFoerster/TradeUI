cmake_minimum_required(VERSION 3.16)
project(TradingDashboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- Dependencies ---

# 1. GLFW
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found, fetching...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
    )
    FetchContent_MakeAvailable(glfw)
endif()

# 2. ImGui (Fetch and place in libs/imgui)
message(STATUS "Fetching ImGui...")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking  # Using docking branch for better dashboard features
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui
)
FetchContent_MakeAvailable(imgui)

# 3. ImPlot (Fetch and place in libs/implot)
message(STATUS "Fetching ImPlot...")
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        master
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libs/implot
)
FetchContent_MakeAvailable(implot)

# 4. Setup Include Directories
include_directories(
    libs/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/implot
    src
)

# 5. Source Files
set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp
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

# 6. Executable
add_executable(TradingDashboard
    src/main.cpp
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
    ${GLAD_SOURCES}
    ${APP_SOURCES}
)

# 7. Linking
if(glfw3_FOUND)
    target_link_libraries(TradingDashboard PRIVATE glfw)
else()
    target_link_libraries(TradingDashboard PRIVATE glfw)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(TradingDashboard PRIVATE pthread dl)
endif()
