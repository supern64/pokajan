set(CMAKE_SYSTEM_NAME Windows)

set(DEVKIT_ROOT "D:/Programming/Other/raylib/w64devkit")
set(ENV{PATH} "${DEVKIT_ROOT}/bin;$ENV{PATH}")
 
set(CMAKE_C_COMPILER   "${DEVKIT_ROOT}/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "${DEVKIT_ROOT}/bin/g++.exe")
set(CMAKE_RC_COMPILER  "${DEVKIT_ROOT}/bin/windres.exe")
set(CMAKE_MAKE_PROGRAM "${DEVKIT_ROOT}/bin/ninja.exe")

# Restrict find_library/find_path/find_package to this devkit's sysroot
# first, so CMake doesn't resolve a library from a different MinGW
# install (e.g. MSYS2) that might also be on PATH.
set(CMAKE_FIND_ROOT_PATH "${DEVKIT_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)