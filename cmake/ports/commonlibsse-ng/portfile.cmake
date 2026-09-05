# Fetch the source over git rather than over GitHub's generated tarballs.
#
# The original port used vcpkg_from_github with a pinned SHA512 of the .tar.gz. Those
# archives are re-compressed by GitHub over time, so the recorded hash eventually stops
# matching and the port fails to download. Fetching the commit with git keeps the same
# pin (the commit id) while letting git verify the content, so this cannot rot the same way.
vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/alandtse/CommonLibVR
    REF e617713b2ae8a927bf925d1ad138cc48ab72e414 # 22/09/24
    FETCH_REF ng
    HEAD_REF ng
)

# Get submodule and copy to extern/ folder (done manually because Vcpkg does not support Git submodules)
vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH_OPENVR
    URL https://github.com/ValveSoftware/openvr
    REF ebdea152f8aac77e9a6db29682b81d762159df7e # 01/09/17
    FETCH_REF master
    HEAD_REF master
)
 file(COPY "${SOURCE_PATH_OPENVR}/" DESTINATION "${SOURCE_PATH}/extern/openvr")
 file(REMOVE_RECURSE "${SOURCE_PATH_OPENVR}/")

 # Configure options to build
vcpkg_configure_cmake(
        SOURCE_PATH "${SOURCE_PATH}"
        PREFER_NINJA
        OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on
)

vcpkg_install_cmake()
vcpkg_cmake_config_fixup(PACKAGE_NAME CommonLibSSE CONFIG_PATH lib/cmake)
vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/extern/openvr/headers/openvr.h" DESTINATION ${CURRENT_PACKAGES_DIR}/include)
file(GLOB CMAKE_CONFIGS "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE/*.cmake")
file(INSTALL ${CMAKE_CONFIGS} DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")
file(INSTALL "${SOURCE_PATH}/cmake/CommonLibSSE.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
