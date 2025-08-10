cmake_minimum_required(VERSION 3.28...3.30)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/bin/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(VCPKG_MANIFEST_DIR "${CMAKE_SOURCE_DIR}/vcpkg")
if(WIN32)
	set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "Vcpkg triplet")
elseif(APPLE)
	set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "Vcpkg triplet")
else()
	set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg triplet")
endif()

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)
list(APPEND CMAKE_VCPKG_PREFIX_PATH ${CMAKE_PREFIX_PATH})

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)
if(WIN32)
	find_package(universalspeech CONFIG REQUIRED)
	target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE psapi Version universalspeech::universalspeech)
	file(GLOB DLLS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/*.dll")
	foreach(DLL IN LISTS DLLS)
		install(FILES "${DLL}" DESTINATION "${CMAKE_PROJECT_NAME}/bin/64bit" COMPONENT Runtime)
		add_custom_command(TARGET "${CMAKE_PROJECT_NAME}" POST_BUILD COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/rundir/$<CONFIG>/${target}" COMMAND "${CMAKE_COMMAND}" -E copy_if_different  "${DLL}" "${CMAKE_CURRENT_BINARY_DIR}/rundir/$<CONFIG>")
	endforeach()
endif()
find_package(fmt CONFIG REQUIRED)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRC_FILES})

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
