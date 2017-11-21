INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_TASH tash)

FIND_PATH(
    TASH_INCLUDE_DIRS
    NAMES tash/api.h
    HINTS $ENV{TASH_DIR}/include
        ${PC_TASH_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    TASH_LIBRARIES
    NAMES gnuradio-tash
    HINTS $ENV{TASH_DIR}/lib
        ${PC_TASH_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TASH DEFAULT_MSG TASH_LIBRARIES TASH_INCLUDE_DIRS)
MARK_AS_ADVANCED(TASH_LIBRARIES TASH_INCLUDE_DIRS)

