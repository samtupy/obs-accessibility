vcpkg_find_acquire_program(PYTHON3)
vcpkg_find_acquire_program(SCONS)

vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO samtupy/UniversalSpeechMSVCStatic
	REF 6af92f89a1b28ebb8cedb357d91f460412c78416
	SHA512 0727a7111b64ef9b67492c5cce8a8e5fb0e865ea827cafa0651e0a861d57c2bdba5770f337c9f0dceb625721503dc7604ebf2f59ec490c327254104281e55f19
	HEAD_REF master
)
if(VCPKG_CRT_LINKAGE STREQUAL "dynamic")
	set(RUNTIME_LIB_SUFFIX "DLL")
endif()
set(ENV{PATH} "$ENV{PATH};${PYTHON3_DIR}")
vcpkg_execute_build_process(
	COMMAND ${SCONS} debug=1 runtime=MultiThreadedDebug${RUNTIME_LIB_SUFFIX}
	WORKING_DIRECTORY ${SOURCE_PATH}
	LOGNAME build-dbg
)
file(INSTALL "${SOURCE_PATH}/UniversalSpeechStatic.lib" DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib)
vcpkg_execute_build_process(
	COMMAND ${SCONS} runtime=MultiThreaded${RUNTIME_LIB_SUFFIX}
	WORKING_DIRECTORY ${SOURCE_PATH}
	LOGNAME build-rel
)
file(INSTALL "${SOURCE_PATH}/UniversalSpeechStatic.lib" DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
file(GLOB DLLS "${SOURCE_PATH}/bin-x64/*.dll")
if(DLLS)
	file(INSTALL ${DLLS} DESTINATION ${CURRENT_PACKAGES_DIR}/bin)
	file(INSTALL ${DLLS} DESTINATION ${CURRENT_PACKAGES_DIR}/debug/bin)
endif()
if(EXISTS "${SOURCE_PATH}/include")
	file(INSTALL "${SOURCE_PATH}/include/" DESTINATION ${CURRENT_PACKAGES_DIR}/include)
endif()
file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
configure_file(
	"${CURRENT_PORT_DIR}/universalspeech-config.cmake.in"
	"${CURRENT_PACKAGES_DIR}/share/${PORT}/universalspeech-config.cmake"
	@ONLY
)
