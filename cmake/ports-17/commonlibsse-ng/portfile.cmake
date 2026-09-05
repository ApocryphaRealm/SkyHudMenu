# The Skyrim 1.7.x build line: CommonLibSSE-NG 7.2.0 from alandtse's `ng` lineage, the first
# CommonLib that reads Address Library's format-5 databases (1.7.99 / 1.7.104) and carries the
# 1.7 struct layouts. Pinned to the v7.2.0 release commit (2026-09-03). VR is off (this line is
# SE/AE), so extern/openvr is not needed. SKSE_SUPPORT_PATCH_SAFETY is off because it vendors
# hde64 through FetchContent at configure time, and vcpkg builds ports fully disconnected.
vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/alandtse/CommonLibSSE-NG
    REF 7a60f4de794095d7b0f8928d1b930a52e9a7da83 # v7.2.0, 2026-09-03
    FETCH_REF ng
    HEAD_REF ng
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=off
        -DSKSE_SUPPORT_XBYAK=on
        -DSKSE_SUPPORT_PATCH_SAFETY=off
        -DENABLE_SKYRIM_SE=on
        -DENABLE_SKYRIM_AE=on
        -DENABLE_SKYRIM_VR=off
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME CommonLibSSE CONFIG_PATH lib/cmake/CommonLibSSE)
vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/cmake/CommonLibSSE.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
file(INSTALL "${SOURCE_PATH}/EXCEPTIONS.md" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
