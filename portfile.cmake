vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO JakubSzwedowicz/Utils
        REF ${VERSION}
        SHA512 0
        HEAD_REF main
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

# Build the project.
vcpkg_cmake_install()

# Remove unnecessary files from the final package.
vcpkg_cmake_config_fixup(
        PACKAGE_NAME "utils"
        CONFIG_PATH "lib/cmake/utils"
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")