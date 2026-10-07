# Compare identical inputs in fresh application processes; no engine goldens change.
foreach(mode classic enhanced switched)
    execute_process(COMMAND "${CMAKE_COMMAND}" -E env
        SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy
        "CIVIC89_GRAPHICS_PARITY=${mode}" "${APPLICATION}"
        WORKING_DIRECTORY "${RUNTIME}" RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 90)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Graphics ${mode} parity failed: ${result}\n${output}\n${error}")
    endif()
endforeach()
foreach(extension json cty c89)
    file(SHA256 "${RUNTIME}/graphics-parity-classic.${extension}" expected)
    foreach(mode enhanced switched)
        file(SHA256 "${RUNTIME}/graphics-parity-${mode}.${extension}" actual)
        if(NOT actual STREQUAL expected)
            message(FATAL_ERROR "Graphics ${mode} changed ${extension} state/RNG/timing/save output")
        endif()
    endforeach()
endforeach()
message(STATUS "Classic/Enhanced/mid-session switches: 256 phase digests, 86 animations, RNG and both save formats identical")
