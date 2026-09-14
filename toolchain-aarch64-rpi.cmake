# Cross-compile toolchain for 64-bit Raspberry Pi OS (aarch64).
#
# Usage:
#   cmake -B build-drm -G Ninja -DBUILD_DRM=ON -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_TOOLCHAIN_FILE=toolchain/aarch64-rpi.cmake \
#         -DRPI_SYSROOT=$HOME/rpi-sysroot
#
# For 32-bit Raspberry Pi OS, swap TOOLCHAIN_PREFIX to arm-linux-gnueabihf
# and CMAKE_SYSTEM_PROCESSOR to arm.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(RPI_SYSROOT "$ENV{HOME}/rpi-sysroot" CACHE PATH "Sysroot copied from the Pi")
set(TOOLCHAIN_PREFIX aarch64-linux-gnu CACHE STRING "Cross toolchain triple")

if(NOT EXISTS "${RPI_SYSROOT}/usr/include")
    message(FATAL_ERROR "RPI_SYSROOT '${RPI_SYSROOT}' does not look like a sysroot")
endif()

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_AR           ${TOOLCHAIN_PREFIX}-ar)
set(CMAKE_RANLIB       ${TOOLCHAIN_PREFIX}-ranlib)
set(CMAKE_STRIP        ${TOOLCHAIN_PREFIX}-strip)

set(CMAKE_SYSROOT      ${RPI_SYSROOT})
set(CMAKE_FIND_ROOT_PATH ${RPI_SYSROOT})

# Look for programs on the host, but headers/libs/packages only in the sysroot.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# pkg-config must read the .pc files in the sysroot and prefix their paths.
set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_LIBDIR}
    "${RPI_SYSROOT}/usr/lib/pkgconfig:${RPI_SYSROOT}/usr/lib/${TOOLCHAIN_PREFIX}/pkgconfig:${RPI_SYSROOT}/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${RPI_SYSROOT}")

# Indirect shared-library dependencies (libGLESv2 -> libdrm, etc.) are resolved
# by the linker against the sysroot, not the host's /usr/lib.
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-Wl,-rpath-link,${RPI_SYSROOT}/lib/${TOOLCHAIN_PREFIX} -Wl,-rpath-link,${RPI_SYSROOT}/usr/lib/${TOOLCHAIN_PREFIX}")