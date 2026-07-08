# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project ("DreamerEngine")

set_source_files_properties("Project/main.cpp" PROPERTIES COMPILE_OPTIONS "/bigobj")

add_executable(DreamerEngine Project/main.cpp)

file(GLOB LUA_SOURCES
    Lib/Lua/*.c
)

file(GLOB IMGUI_SOURCES
    Lib/imgui/*.cpp
)

target_include_directories(DreamerEngine
    PRIVATE Project
    PRIVATE Lib
)

add_library(Lua STATIC ${LUA_SOURCES})
add_library(ImGUI STATIC ${IMGUI_SOURCES})
target_link_libraries(DreamerEngine PRIVATE Lua)
target_link_libraries(DreamerEngine PRIVATE ImGUI)

target_include_directories(ImGUI
    PRIVATE Lib
)

set(OpenGL_GL_PREFERENCE "LEGACY")

if (UNIX)
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)

    target_link_libraries(DreamerEngine
        PRIVATE GLEW
        PRIVATE glfw
        PRIVATE OpenGL::GL
    )
endif(UNIX)

if (WIN32)
    target_link_libraries(DreamerEngine
        PRIVATE ${CMAKE_SOURCE_DIR}/Lib/glfw3.lib
        PRIVATE ${CMAKE_SOURCE_DIR}/Lib/glew32s.lib
    )
endif(WIN32)

add_custom_command(
    TARGET DreamerEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Engine
        $<TARGET_FILE_DIR:DreamerEngine>/Engine
)