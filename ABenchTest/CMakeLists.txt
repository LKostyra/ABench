MESSAGE(STATUS "Generating Makefile for ABenchTest")

FILE(GLOB ABENCHTEST_SOURCES          *.cpp)
FILE(GLOB ABENCHTEST_HEADERS          *.hpp)
FILE(GLOB ABENCHTEST_TEST_SOURCES     Tests/*.cpp)
FILE(GLOB ABENCHTEST_TEST_HEADERS     Tests/*.hpp)

# Tested modules and their dependencies
SET(ABENCHTEST_MODULES_SOURCES        ${ABENCH_DIRECTORY}/Common/FBXFile.cpp
                                      ${ABENCH_DIRECTORY}/Common/Linux/Logger.cpp
                                      ${ABENCH_DIRECTORY}/Common/Linux/Common.cpp
                                      ${ABENCH_DIRECTORY}/Common/Linux/Timer.cpp
                                      ${ABENCH_DIRECTORY}/Common/Linux/FS.cpp
                                      ${ABENCH_DIRECTORY}/Common/Linux/Window.cpp
                                      ${ABENCH_DIRECTORY}/Math/Vector.cpp
                                      )

SET(ABENCHTEST_MODULES_HEADERS        ${ABENCH_DIRECTORY}/Common/FBXFile.hpp
                                      ${ABENCH_DIRECTORY}/Common/Logger.hpp
                                      ${ABENCH_DIRECTORY}/Common/Common.hpp
                                      ${ABENCH_DIRECTORY}/Common/FS.hpp
                                      ${ABENCH_DIRECTORY}/Common/Window.hpp
                                      ${ABENCH_DIRECTORY}/Math/Vector.hpp
                                      )

PKG_CHECK_MODULES(ABENCHTEST_PKG_DEPS REQUIRED
                  xcb
                  )

INCLUDE_DIRECTORIES(${ABENCH_ROOT_DIRECTORY} ${ABENCHTEST_DIRECTORY} ${ABENCH_DIRECTORY}
                    ${ABENCH_DEPS_FBXSDK_INCLUDE_DIRECTORY}
                    ${ABENCH_DEPS_GTEST_INCLUDE_DIRECTORY}
                    ${ABENCHTEST_PKG_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${ABENCH_LIB_DIRECTORY}
                 ${ABENCH_DEPS_FBXSDK_LIB_DIRECTORY}
                 ${ABENCHTEST_PKG_DEPS_LIBRARY_DIRS})

ADD_EXECUTABLE(ABenchTest
               ${ABENCHTEST_SOURCES} ${ABENCHTEST_HEADERS}
               ${ABENCHTEST_TEST_SOURCES} ${ABENCHTEST_TEST_HEADERS}
               ${ABENCHTEST_MODULES_SOURCES} ${ABENCHTEST_MODULES_HEADERS})

ADD_DEPENDENCIES(ABenchTest gtest)

# TODO uncomment when glslang will be added
#TARGET_LINK_LIBRARIES(ABench glslang hlsl SPIRV OGLCompiler OSDependent)
TARGET_LINK_LIBRARIES(ABenchTest gtest fbxsdk dl pthread
                      ${ABENCHTEST_PKG_DEPS_LIBRARIES})

ADD_CUSTOM_COMMAND(TARGET ABenchTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ABenchTest> ${ABENCH_OUTPUT_DIRECTORY}/${targetfile})
