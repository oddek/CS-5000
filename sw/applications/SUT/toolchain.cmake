#-------------------------------------------------------------------------
# System name used by conditions across project
#-------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_SYSROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../buildroot/output/staging)
set(CMAKE_STAGING_PREFIX  ${CMAKE_CURRENT_SOURCE_DIR}/../../rootfs/root)

#-------------------------------------------------------------------------
# Find and set correct compiler
#-------------------------------------------------------------------------
set(TOOLCHAIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../toolchain/x-tools/arm-cortexa9_neon-linux-gnueabihf/bin")
set(TOOLCHAIN_PREFIX arm-cortexa9_neon-linux-gnueabihf)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}-objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}-objdump)
set(CMAKE_OBJSIZE ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}-size)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#-------------------------------------------------------------------------
# Set compile options
#-------------------------------------------------------------------------
add_compile_options(
        -Wall
        -Wno-main
        -Wundef
        -pedantic
        -Wno-variadic-macros
        -Werror
        -Wfatal-errors
        -Wl,--relax,--gc-sections
        -g
        -Wno-psabi 
)