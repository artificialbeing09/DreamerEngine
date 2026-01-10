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

project ("Concise")

add_executable(Concise Project/main.cpp)

file(GLOB LUA_SOURCES
    Lib/Lua/*.c
)

add_library(Lua STATIC ${LUA_SOURCES})
target_link_libraries(Concise PRIVATE Lua)

target_include_directories(Concise
    PRIVATE Project
    PRIVATE Lib
)

set(OpenGL_GL_PREFERENCE "LEGACY")

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(Concise
    #PRIVATE ${CMAKE_SOURCE_DIR}/Lib/glfw3.lib
    #PRIVATE ${CMAKE_SOURCE_DIR}/Lib/glew32s.lib
    PRIVATE GLEW
    PRIVATE glfw
    PRIVATE OpenGL::GL
)

add_custom_command(
    TARGET Concise POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Engine
        $<TARGET_FILE_DIR:Concise>/Engine
)