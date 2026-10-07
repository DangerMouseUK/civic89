cmake_minimum_required(VERSION 4.2)
project(Civic89 VERSION 0.9.0 LANGUAGES CXX)
set(CIVIC89_PRERELEASE "beta.2")

if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Civic 89 supports Windows x64/ARM64 with MSVC.")
endif()
option(CIVIC89_BUILD_APPLICATION "Build the native SDL application and presentation tests" ON)
option(CIVIC89_ENABLE_ASAN "Instrument engine tests with AddressSanitizer" OFF)
if(CIVIC89_BUILD_APPLICATION AND NOT VCPKG_MANIFEST_MODE)
    message(FATAL_ERROR "Use a Civic 89 application preset with the explicit vcpkg manifest toolchain.")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false;VcpkgEnableManifest=false")
include(cmake/BuildProvenance.cmake)
if(CIVIC89_ENABLE_ASAN)
    if(NOT CIVIC89_ARCH STREQUAL "x64")
        message(FATAL_ERROR "The current MSVC AddressSanitizer presets are x64 only.")
    endif()
    add_compile_options(/fsanitize=address /Zi)
    add_link_options(/INCREMENTAL:NO /DEBUG)
endif()

include(cmake/InheritedSources.cmake)
include(cmake/EngineSources.cmake)
add_library(civic89_engine STATIC ${CIVIC89_ENGINE_SOURCES})
target_include_directories(civic89_engine PUBLIC src)
target_compile_options(civic89_engine PRIVATE /W4 /sdl /permissive- "$<$<CONFIG:Release>:/fp:fast>")
set_target_properties(civic89_engine PROPERTIES VS_GLOBAL_VcpkgEnabled false VS_GLOBAL_VcpkgEnableManifest false)
add_executable(civic89_runner src/EngineRunner.cpp)
target_link_libraries(civic89_runner PRIVATE civic89_engine)
target_compile_options(civic89_runner PRIVATE /W4 /sdl /permissive-)

add_library(civic89_storage STATIC src/WindowsFileStorage.cpp)
target_include_directories(civic89_storage PUBLIC src)
target_compile_options(civic89_storage PRIVATE /W4 /sdl /permissive-)
set_target_properties(civic89_storage PROPERTIES VS_GLOBAL_VcpkgEnabled false VS_GLOBAL_VcpkgEnableManifest false)

include(CTest)
if(BUILD_TESTING)
    add_executable(civic89_ui_tests tests/UiModels.cpp src/UiSettings.cpp src/GraphicsSettings.cpp src/OverlayModel.cpp)
    target_link_libraries(civic89_ui_tests PRIVATE civic89_engine civic89_storage)
    target_compile_options(civic89_ui_tests PRIVATE /W4 /sdl /permissive-)
    add_test(NAME ui-overlay-preferences COMMAND civic89_ui_tests)
    set_tests_properties(ui-overlay-preferences PROPERTIES TIMEOUT 30)
    add_executable(civic89_camera_tests tests/CameraPresentation.cpp src/Camera2D.cpp src/DisplaySettings.cpp)
    target_link_libraries(civic89_camera_tests PRIVATE civic89_engine civic89_storage)
    target_compile_options(civic89_camera_tests PRIVATE /W4 /sdl /permissive-)
    add_test(NAME camera-layout-cadence-settings COMMAND civic89_camera_tests)
    set_tests_properties(camera-layout-cadence-settings PROPERTIES TIMEOUT 30)
    add_executable(civic89_tests tests/EngineTests.cpp)
    target_link_libraries(civic89_tests PRIVATE civic89_engine civic89_storage)
    target_compile_options(civic89_tests PRIVATE /W4 /sdl /permissive-)
    target_compile_definitions(civic89_tests PRIVATE "$<$<CONFIG:Release>:CIVIC89_FAST_FLOAT>")
    add_executable(civic89_faithfulness_tests tests/Faithfulness.cpp)
    target_link_libraries(civic89_faithfulness_tests PRIVATE civic89_engine)
    target_compile_options(civic89_faithfulness_tests PRIVATE /W4 /sdl /permissive-)
    add_test(NAME original-mechanics COMMAND civic89_faithfulness_tests "${CMAKE_SOURCE_DIR}")
    set_tests_properties(original-mechanics PROPERTIES TIMEOUT 60)
    add_test(NAME fidelity-source-contract COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/VerifyFidelity.cmake")
    add_executable(civic89_io_tests tests/FunctionalIo.cpp src/RecoveryStore.cpp src/DiagnosticLog.cpp)
    target_link_libraries(civic89_io_tests PRIVATE civic89_engine civic89_storage)
    target_compile_options(civic89_io_tests PRIVATE /W4 /sdl /permissive-)
    add_test(NAME functional-persistence COMMAND civic89_io_tests)
    set_tests_properties(functional-persistence PROPERTIES TIMEOUT 60)
    add_executable(civic89_compatibility_tests tests/RulesetCompatibility.cpp src/RecoveryStore.cpp)
    target_link_libraries(civic89_compatibility_tests PRIVATE civic89_engine civic89_storage)
    target_compile_options(civic89_compatibility_tests PRIVATE /W4 /sdl /permissive-)
    add_test(NAME ruleset-save-compatibility COMMAND civic89_compatibility_tests "${CMAKE_SOURCE_DIR}")
    set_tests_properties(ruleset-save-compatibility PROPERTIES TIMEOUT 60)
    add_test(NAME engine-smoke COMMAND civic89_tests --smoke "${CMAKE_SOURCE_DIR}")
    set_tests_properties(engine-smoke PROPERTIES TIMEOUT 60)
    foreach(scenario RANGE 1 8)
        foreach(ticks IN ITEMS 0 16 1024)
            add_test(NAME parity-scenario-${scenario}-${ticks}
                COMMAND civic89_tests --golden "${CMAKE_SOURCE_DIR}" ${scenario} ${ticks})
        endforeach()
    endforeach()
    add_test(NAME parity-long-run COMMAND civic89_tests --golden "${CMAKE_SOURCE_DIR}" 6 16384)
    set_tests_properties(parity-long-run PROPERTIES TIMEOUT 120)
    add_test(NAME runner-scenario COMMAND civic89_runner --scenario 6 --scenario-dir "${CMAKE_SOURCE_DIR}/scenarios" --seed 12345 --ticks 1024)
    set_tests_properties(runner-scenario PROPERTIES PASS_REGULAR_EXPRESSION "digest_v1=0f23de43b43a9e52")
    add_test(NAME runner-city COMMAND civic89_runner --city "${CMAKE_SOURCE_DIR}/scenarios/snro.666" --seed 12345 --ticks 1024)
    set_tests_properties(runner-city PROPERTIES PASS_REGULAR_EXPRESSION "city=snro ticks=1024 city_time=64")
    add_test(NAME runner-generated-city COMMAND civic89_runner --generate --seed 2468 --ticks 1024)
    set_tests_properties(runner-generated-city PROPERTIES PASS_REGULAR_EXPRESSION "ticks=1024 city_time=64")
    # Captured from pristine merged M6 fa3534b before M7 engine changes. Separate
    # processes prevent inherited session globals from obscuring mode parity.
    foreach(mode IN ITEMS classic enhanced)
        foreach(ticks IN ITEMS 1024 16384)
            if(ticks EQUAL 1024)
                set(expected_digest 06f70d4804f2c435)
            else()
                set(expected_digest 4fadfeaabc62970d)
            endif()
            add_test(NAME parity-generated-${mode}-${ticks}
                COMMAND civic89_runner --generate --mode ${mode} --seed 2468 --ticks ${ticks})
            set_tests_properties(parity-generated-${mode}-${ticks} PROPERTIES
                PASS_REGULAR_EXPRESSION "ruleset=${mode}/1 digest_v1=${expected_digest}" TIMEOUT 120)
        endforeach()
    endforeach()
    add_test(NAME runner-invalid-mode COMMAND civic89_runner --generate --mode enhanced-v2)
    add_test(NAME runner-enhanced-scenario COMMAND civic89_runner --scenario 6 --mode enhanced)
    add_test(NAME runner-city-mode-conflict COMMAND civic89_runner --city "${CMAKE_SOURCE_DIR}/scenarios/snro.666" --mode enhanced)
    set_tests_properties(runner-invalid-mode runner-enhanced-scenario runner-city-mode-conflict PROPERTIES WILL_FAIL TRUE TIMEOUT 10)
    add_test(NAME runner-invalid-argument COMMAND civic89_runner --scenario 1junk)
    set_tests_properties(runner-invalid-argument PROPERTIES WILL_FAIL TRUE TIMEOUT 10)
    add_test(NAME runner-missing-city COMMAND civic89_runner --city "${CMAKE_BINARY_DIR}/missing.cty")
    set_tests_properties(runner-missing-city PROPERTIES WILL_FAIL TRUE TIMEOUT 10)
    add_test(NAME engine-boundary COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" "-DRUNNER=$<TARGET_FILE:civic89_runner>" "-DDUMP_TOOL=${CMAKE_LINKER}" -P "${CMAKE_SOURCE_DIR}/cmake/VerifyEngineBoundary.cmake")
endif()


if(CIVIC89_BUILD_APPLICATION)
    enable_language(RC)
    find_package(SDL3 CONFIG REQUIRED)
    find_package(SDL3_image CONFIG REQUIRED)
    find_package(SDL3_ttf CONFIG REQUIRED)
    find_package(SDL3_mixer CONFIG REQUIRED)
    find_package(nfd CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)
    set(CIVIC89_APPLICATION_SOURCES ${CIVIC89_INHERITED_SOURCES})
    list(REMOVE_ITEM CIVIC89_APPLICATION_SOURCES ${CIVIC89_ENGINE_SOURCES})
    configure_file(packaging/civic89.rc.in "${CMAKE_BINARY_DIR}/generated/civic89.rc" @ONLY)
    add_executable(civic89 ${CIVIC89_APPLICATION_SOURCES} "${CMAKE_BINARY_DIR}/generated/civic89.rc")
    target_include_directories(civic89 PRIVATE "${CMAKE_BINARY_DIR}/generated")
    target_compile_definitions(civic89 PRIVATE CIVIC89_CMAKE_BUILD)
    target_link_libraries(civic89 PRIVATE civic89_engine
        SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf SDL3_mixer::SDL3_mixer
        nfd::nfd nlohmann_json::nlohmann_json Shlwapi)
    target_compile_definitions(civic89 PRIVATE WINDOWS _CONSOLE UNICODE _UNICODE)
    target_compile_options(civic89 PRIVATE /W4 /sdl /permissive- "$<$<CONFIG:Release>:/fp:fast>" "$<$<CONFIG:Release>:/Zi>")
    target_link_options(civic89 PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
    set_property(TARGET civic89 PROPERTY WIN32_EXECUTABLE "$<NOT:$<CONFIG:Debug>>")
    set_property(TARGET civic89 PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set_property(TARGET civic89 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:civic89>")
    set_target_properties(civic89 PROPERTIES VS_GLOBAL_VcpkgEnabled false VS_GLOBAL_VcpkgEnableManifest false)
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

    if(BUILD_TESTING)
        add_executable(civic89_runtime_tests
            tests/RuntimeSmoke.cpp src/Font.cpp src/GameDataLoader.cpp
        )
        target_include_directories(civic89_runtime_tests PRIVATE src)
        target_link_libraries(civic89_runtime_tests PRIVATE
            SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf nlohmann_json::nlohmann_json
        )
        target_compile_options(civic89_runtime_tests PRIVATE /W4 /sdl /permissive-)
        set_target_properties(civic89_runtime_tests PROPERTIES
            VS_GLOBAL_VcpkgEnabled false VS_GLOBAL_VcpkgEnableManifest false
        )
        add_dependencies(civic89_runtime_tests civic89_runtime_assets)
        add_test(NAME runtime-assets-and-data COMMAND civic89_runtime_tests)
        set_tests_properties(runtime-assets-and-data PROPERTIES
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:civic89_runtime_tests>"
            ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_RENDER_DRIVER=software;SDL_AUDIODRIVER=dummy"
            TIMEOUT 60
        )
        add_test(NAME inherited-source-list COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/VerifySourceParity.cmake"
        )

        add_executable(civic89_audio_tests tests/AudioBackend.cpp src/AudioManager.cpp src/DiagnosticLog.cpp)
        target_include_directories(civic89_audio_tests PRIVATE src)
        target_link_libraries(civic89_audio_tests PRIVATE SDL3::SDL3 SDL3_mixer::SDL3_mixer)
        target_compile_options(civic89_audio_tests PRIVATE /W4 /sdl /permissive-)
        add_test(NAME mixer-backend COMMAND civic89_audio_tests)
        set_tests_properties(mixer-backend PROPERTIES ENVIRONMENT "SDL_AUDIODRIVER=dummy" TIMEOUT 60)

        # Exercise typed audio/earthquake helpers without constructing the game/UI.
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

        # Exercise native lifecycle code with the same application sources and dummy SDL.
        add_executable(civic89_milestone_tests ${CIVIC89_APPLICATION_SOURCES} tests/MilestoneLifecycle.cpp tests/WindowCameraRendering.cpp tests/ModernUiAcceptance.cpp tests/GraphicsAcceptance.cpp)
        target_include_directories(civic89_milestone_tests PRIVATE src)
        target_include_directories(civic89_milestone_tests PRIVATE "${CMAKE_BINARY_DIR}/generated")
        target_compile_definitions(civic89_milestone_tests PRIVATE CIVIC89_CMAKE_BUILD)
        target_compile_definitions(civic89_milestone_tests PRIVATE CIVIC89_MILESTONE_TESTS WINDOWS UNICODE _UNICODE)
        target_link_libraries(civic89_milestone_tests PRIVATE civic89_engine
            SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf SDL3_mixer::SDL3_mixer
            nfd::nfd nlohmann_json::nlohmann_json Shlwapi
        )
        target_compile_options(civic89_milestone_tests PRIVATE /W4 /sdl /permissive-)
        set_target_properties(civic89_milestone_tests PROPERTIES
            VS_GLOBAL_VcpkgEnabled false VS_GLOBAL_VcpkgEnableManifest false
        )
        add_dependencies(civic89_milestone_tests civic89_runtime_assets)
        add_test(NAME milestone-lifecycle COMMAND civic89_milestone_tests)
        add_test(NAME graphics-parity COMMAND "${CMAKE_COMMAND}"
            "-DAPPLICATION=$<TARGET_FILE:civic89_milestone_tests>"
            "-DRUNTIME=$<TARGET_FILE_DIR:civic89_milestone_tests>"
            -P "${CMAKE_SOURCE_DIR}/cmake/VerifyGraphicsParity.cmake")
        set_tests_properties(graphics-parity PROPERTIES TIMEOUT 240 RUN_SERIAL TRUE)
        set_tests_properties(milestone-lifecycle PROPERTIES
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:civic89_milestone_tests>"
            ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_RENDER_DRIVER=software;SDL_AUDIODRIVER=dummy"
            TIMEOUT 180
        )
        add_test(NAME no-legacy-command-bridge COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/VerifyNoLegacyCommands.cmake")
        add_test(NAME invalid-scenario-argument COMMAND civic89 --scenario 9)
        set_tests_properties(invalid-scenario-argument PROPERTIES WILL_FAIL TRUE TIMEOUT 10)
        add_test(NAME malformed-scenario-argument COMMAND civic89 --scenario 1junk)
        set_tests_properties(malformed-scenario-argument PROPERTIES WILL_FAIL TRUE TIMEOUT 10)
        add_test(NAME invalid-mode-argument COMMAND civic89 --mode enhanced-v2)
        add_test(NAME enhanced-scenario-argument COMMAND civic89 --scenario 6 --mode enhanced)
        set_tests_properties(invalid-mode-argument enhanced-scenario-argument PROPERTIES WILL_FAIL TRUE TIMEOUT 10)
    endif()

    # Install only the application, pinned dependencies and inventoried assets.
    install(TARGETS civic89 RUNTIME DESTINATION .)
    install(FILES "${CMAKE_BINARY_DIR}/build-info.json" DESTINATION .)
    install(DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<$<CONFIG:Debug>:debug/>bin/"
        DESTINATION . FILES_MATCHING PATTERN "*.dll"
    )
    install(CODE "
        set(SOURCE_DIR \"${CMAKE_SOURCE_DIR}\")
        set(DESTINATION \"\${CMAKE_INSTALL_PREFIX}\")
        set(VCPKG_SHARE_DIR \"${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share\")
        include(\"${CMAKE_SOURCE_DIR}/cmake/StageRuntime.cmake\")
    ")
endif()

if(CIVIC89_ENABLE_ASAN)
    get_filename_component(msvc_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(asan_runtime "${msvc_bin}/clang_rt.asan_dynamic-x86_64.dll")
    if(NOT EXISTS "${asan_runtime}")
        message(FATAL_ERROR "MSVC AddressSanitizer runtime is missing: ${asan_runtime}")
    endif()
    foreach(target IN ITEMS civic89_runner civic89_tests civic89_faithfulness_tests civic89_camera_tests civic89_ui_tests civic89_io_tests civic89_compatibility_tests civic89 civic89_runtime_tests civic89_legacy_tests civic89_milestone_tests civic89_audio_tests)
        if(TARGET ${target})
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${asan_runtime}" "$<TARGET_FILE_DIR:${target}>"
                VERBATIM)
        endif()
    endforeach()
endif()
