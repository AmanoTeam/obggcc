set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "s390x")

set(TARGET_GLIBC "2.24")
set(TOOLCHAIN_PREFIX "${CMAKE_SYSTEM_PROCESSOR}-unknown-linux-gnu")

set(TOOLCHAIN_PATH "$ENV{OBGGCC_HOME}")

if ("${TOOLCHAIN_PATH}" STREQUAL "")
	set(TOOLCHAIN_PATH "${CMAKE_CURRENT_LIST_DIR}/../..")
endif()

set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}${TARGET_GLIBC}-gcc" CACHE FILEPATH "cc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}${TARGET_GLIBC}-g++" CACHE FILEPATH "c++")
set(CMAKE_AR "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-ar" CACHE FILEPATH "ar")
set(CMAKE_RANLIB "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-ranlib" CACHE FILEPATH "ranlib")
set(CMAKE_STRIP "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-strip" CACHE FILEPATH "strip")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-objcopy" CACHE FILEPATH "objcopy")
set(CMAKE_NM "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-nm" CACHE FILEPATH "nm")

set(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}${TARGET_GLIBC}" CACHE FILEPATH "sysroot")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
