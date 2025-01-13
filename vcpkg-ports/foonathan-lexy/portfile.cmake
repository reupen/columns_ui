vcpkg_minimum_required(VERSION 2022-10-12) # for ${VERSION}

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO foonathan/lexy
    REF "34d2adf74a2b25b6bdd760a3bbb931f3fd5e60cd"
    SHA512 f47f932b9a573009a97f9ecfa54614e1087df07c10c8dec5cf1466820ad0f2cfb2d407d302e01b3758a0dc2b434833eb198f1889b643429c927d583e69c6c595
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DLEXY_BUILD_BENCHMARKS=OFF
        -DLEXY_BUILD_EXAMPLES=OFF
        -DLEXY_BUILD_TESTS=OFF
        -DLEXY_BUILD_DOCS=OFF
        -DLEXY_BUILD_PACKAGE=OFF
        -DLEXY_ENABLE_INSTALL=ON
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME lexy
    CONFIG_PATH lib/cmake/lexy
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
