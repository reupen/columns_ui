vcpkg_minimum_required(VERSION 2022-10-12) # for ${VERSION}

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO foonathan/lexy
    REF "3f203d88af8729b5972782bccdf0d9ed932cbd7f"
    SHA512 5375e1fdc028649caaa2b6bfecc307227d27d9ea00111b30e043721b786dc6aff78280d350640517c5c9b175667c7c3be9a07c7b3a98c9c35b93a237bb534cbb
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
