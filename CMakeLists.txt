cmake_minimum_required(VERSION 3.21)

########################################################################################################################
## Define project
########################################################################################################################

project(
	SkyHudMenu
	VERSION 1.0.2
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

# CONFIGURE_DEPENDS so that adding a source file re-runs the glob on the next build
# instead of silently leaving the new file out of the link.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "source/*.cpp")

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${headers}
		${sources}
)

########################################################################################################################
## Configure target DLL
########################################################################################################################

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(spdlog REQUIRED CONFIG)
find_package(directxtk CONFIG REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

# Build line: 1 (SE/AE 1.6, CommonLibSSE-NG 3.7) or 17 (Skyrim 1.7.x, CommonLibSSE-NG 7.2).
# The build-relwithdebinfo-17 preset sets this to 17; source #if-branches the NG 3.7 -> 7.2 API renames.
set(RUNTIME_LINE "1" CACHE STRING "Build line: 1 (SE/AE 1.6) or 17 (Skyrim 1.7.x)")
target_compile_definitions(${PROJECT_NAME} PRIVATE RUNTIME_LINE=${RUNTIME_LINE})

########################################################################################################################
## Debug symbols (CLAUDE.md rule 43) - stated on the target, never left to a default
########################################################################################################################
#
# The preset chain leaves CMAKE_CXX_FLAGS_RELWITHDEBINFO EMPTY and the link flags as
# "/INCREMENTAL:NO /LTCG" - no /Zi, no /DEBUG. RelWithDebInfo therefore produces a DLL with NO
# CodeView (RSDS) record at all, and the PDB beside it is whatever the previous build left there.
# Caught 2026-08-26 on IshSoulsToPerks, only because the packaging script refuses a DLL it cannot
# prove a PDB belongs to.
#
# cmake_minimum_required here is 3.21, where CMP0141 is OLD and
# CMAKE_MSVC_DEBUG_INFORMATION_FORMAT is not consulted - so unlike SureOfStealing (3.29), nothing
# rescues this implicitly.
#
# /DEBUG implies /OPT:NOREF, so /OPT:REF and /OPT:ICF are restated to keep Release-grade linking.
# Do not remove these on the assumption that RelWithDebInfo "obviously" has debug info.
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /Zi)
	target_link_options(${PROJECT_NAME} PRIVATE /DEBUG:FULL /OPT:REF /OPT:ICF)

	########################################################################################
	## Do not ship the build machine's absolute paths inside the DLL
	########################################################################################
	#
	# Before this, released DLLs contained ~30 copies of "D:\Claude output\..." - the PDB path
	# the linker records, plus one absolute source path per logging call site, because SKSE::log
	# captures std::source_location and __FILE__ is whatever the compiler was handed. That leaks
	# the build tree's layout to every downloader for no benefit, and long absolute developer
	# paths in an unsigned DLL are a plausible contributor to antivirus heuristic scoring (see the
	# Wacatac!ml false-positive report, 2026-08-27).
	#
	# /PDBALTPATH:%_PDB% records only the PDB's FILENAME in the debug directory instead of its full
	# path. Safe for us specifically: Crash Logger resolves symbols from the PDB sitting beside the
	# DLL, which is exactly how rule 43 ships them - verified still pairing after the change.
	#
	# /d1trimfile: strips a prefix from __FILE__ at compile time. The prefix must be in native
	# (backslash) form to match what the compiler actually received, and deliberately has NO
	# trailing separator - a trailing backslash immediately before the closing quote would escape
	# that quote and mangle the argument.
	file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" PROJECT_NATIVE_DIR)
	target_compile_options(${PROJECT_NAME} PRIVATE "/d1trimfile:${PROJECT_NATIVE_DIR}")
	target_link_options(${PROJECT_NAME} PRIVATE /PDBALTPATH:%_PDB%)
endif()

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		spdlog::spdlog_header_only
		Microsoft::DirectXTK
)

if (MSVC)
	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF>"
	)
endif()

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		include/PCH.h
)

target_sources(
	${PROJECT_NAME}
	INTERFACE
	"$<BUILD_INTERFACE:CommonLibSSE.natvis>"
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

########################################################################################################################
## Pre-link clean-up
########################################################################################################################

file(GLOB PRE_LINK_CLEAN_DIRS "${VCPKG_INSTALLED_DIR}/*")
list(REMOVE_ITEM PRE_LINK_CLEAN_DIRS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")

foreach(PRE_LINK_CLEAN_DIR ${PRE_LINK_CLEAN_DIRS})
	add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
		COMMAND ${CMAKE_COMMAND} -E rm -rf "${PRE_LINK_CLEAN_DIR}"
	)
endforeach()

########################################################################################################################
## Automatic plugin deployment
########################################################################################################################

set(SKYRIM_SE_DIR "C:/Program Files (x86)/Steam/steamapps/common/Skyrim Special Edition")
set(SKYRIM_VR_DIR "C:/Program Files (x86)/Steam/steamapps/common/Skyrim VR")

if ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "")
	set(COPY_SE YES)
	set(COPY_VR YES)
elseif ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "-UENABLE_SKYRIM_VR")
	set(COPY_SE YES)
	set(COPY_VR NO)
elseif ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "-UENABLE_SKYRIM_SE -UENABLE_SKYRIM_AE")
	set(COPY_SE NO)
	set(COPY_VR YES)
endif()

# Only auto-deploy when the game folder actually exists on this machine. Otherwise the
# build output is collected by package.bat instead.
if (COPY_SE AND EXISTS "${SKYRIM_SE_DIR}/Data/SKSE/Plugins")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
		"${SKYRIM_SE_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
	)
endif()
if (COPY_VR AND EXISTS "${SKYRIM_VR_DIR}/Data/SKSE/Plugins")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
		"${SKYRIM_VR_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
	)
endif()
