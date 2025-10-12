vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO JakubSzwedowicz/Utils
        REF ${VERSION}
        SHA512 bf5eb1bfef74e42a402f80bceae2da7e0e516c8364f6a9c989b8bb57c07e293747c838e9ea1dfc4b3bdc938c751bd9aa79e6132f22e5604d0251438f39504846
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