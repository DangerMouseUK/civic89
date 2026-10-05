cmake_minimum_required(VERSION 4.2)

project(Civic89 VERSION 0.1.0 LANGUAGES CXX RC)

if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "The bootstrap supports Windows x64 with MSVC.")
endif()
if(NOT VCPKG_MANIFEST_MODE)
    message(FATAL_ERROR "Use a Civic 89 preset with the explicit vcpkg manifest toolchain.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include(cmake/InheritedSources.cmake)
add_executable(civic89 ${CIVIC89_INHERITED_SOURCES} micropolis-sdlpp.rc)
target_link_libraries(civic89 PRIVATE
    SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf
    nfd::nfd nlohmann_json::nlohmann_json Shlwapi
)
target_compile_definitions(civic89 PRIVATE WINDOWS _CONSOLE UNICODE _UNICODE)
target_compile_options(civic89 PRIVATE
    /W4 /sdl /permissive-
    "$<$<CONFIG:Release>:/fp:fast>"
    "$<$<CONFIG:Release>:/Zi>"
)
target_link_options(civic89 PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
set_property(TARGET civic89 PROPERTY WIN32_EXECUTABLE "$<NOT:$<CONFIG:Debug>>")
set_property(TARGET civic89 PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set_property(TARGET civic89 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:civic89>")
# Disable inherited global MSBuild integration on every generated project.
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false;VcpkgEnableManifest=false")
set_target_properties(civic89 PROPERTIES
    VS_GLOBAL_VcpkgEnabled false VS_GLOBAL_VcpkgEnableManifest false
)

add_custom_target(civic89_runtime_assets ALL
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
        "-DDESTINATION=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        "-DVCPKG_SHARE_DIR=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share"
        -P "${CMAKE_SOURCE_DIR}/cmake/StageRuntime.cmake"
    COMMENT "Stage the inventoried runtime assets and licence notices"
    VERBATIM
)
add_dependencies(civic89 civic89_runtime_assets)
add_custom_target(run
    COMMAND "$<TARGET_FILE:civic89>"
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:civic89>"
    DEPENDS civic89
    USES_TERMINAL
)

include(CTest)
if(BUILD_TESTING)
    add_executable(civic89_tests
        tests/RuntimeSmoke.cpp src/Font.cpp src/GameDataLoader.cpp
    )
    target_include_directories(civic89_tests PRIVATE src)
    target_link_libraries(civic89_tests PRIVATE
        SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf nlohmann_json::nlohmann_json
    )
    target_compile_options(civic89_tests PRIVATE /W4 /sdl /permissive-)
    set_target_properties(civic89_tests PROPERTIES
        VS_GLOBAL_VcpkgEnabled false VS_GLOBAL_VcpkgEnableManifest false
    )
    add_dependencies(civic89_tests civic89_runtime_assets)
    add_test(NAME runtime-assets-and-data COMMAND civic89_tests)
    set_tests_properties(runtime-assets-and-data PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:civic89_tests>"
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_RENDER_DRIVER=software"
        TIMEOUT 60
    )
    add_test(NAME inherited-source-list COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/VerifySourceParity.cmake"
    )

    # Characterize the actual legacy bridge without constructing the game/UI.
    add_executable(civic89_legacy_tests
        tests/LegacyBridge.cpp src/w_tk.cpp src/w_sound.cpp
    )
    target_include_directories(civic89_legacy_tests PRIVATE src)
    target_link_libraries(civic89_legacy_tests PRIVATE SDL3::SDL3)
    target_compile_options(civic89_legacy_tests PRIVATE /W4 /sdl /permissive-)
    set_target_properties(civic89_legacy_tests PROPERTIES
        VS_GLOBAL_VcpkgEnabled false VS_GLOBAL_VcpkgEnableManifest false
    )
    add_test(NAME legacy-audio-and-earthquake COMMAND civic89_legacy_tests)
    set_tests_properties(legacy-audio-and-earthquake PROPERTIES TIMEOUT 10)
endif()

# Install only the application, pinned dependencies and inventoried assets.
install(TARGETS civic89 RUNTIME DESTINATION .)
install(DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<$<CONFIG:Debug>:debug/>bin/"
    DESTINATION . FILES_MATCHING PATTERN "*.dll"
)
install(CODE "
    set(SOURCE_DIR \"${CMAKE_SOURCE_DIR}\")
    set(DESTINATION \"\${CMAKE_INSTALL_PREFIX}\")
    set(VCPKG_SHARE_DIR \"${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share\")
    include(\"${CMAKE_SOURCE_DIR}/cmake/StageRuntime.cmake\")
")
