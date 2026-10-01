add_executable(nes_emulator_wasm
        WasmApp.cpp
        WasmApp.h
        Bindings.cpp
        main_wasm.cpp)

# Emscripten's own SDL3 port (vcpkg's SDL3::SDL3 has no Emscripten triplet).
target_compile_options(nes_emulator_wasm PRIVATE -sUSE_SDL=3)
target_link_libraries(nes_emulator_wasm PRIVATE nes_core nes_frontend nes_warnings)
target_link_options(nes_emulator_wasm PRIVATE
        -sUSE_SDL=3
        -sALLOW_MEMORY_GROWTH=1
        -sMODULARIZE=1
        -sEXPORT_NAME=NesModule
        -sENVIRONMENT=web
        -lembind)

# Emit the .js/.wasm into build-wasm/web/ next to the browser shell, so that directory is
# directly servable.
set(NES_WEB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/web)
set_target_properties(nes_emulator_wasm PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${NES_WEB_OUTPUT_DIR})

set(NES_WEB_SHELL_FILES
        ${PROJECT_SOURCE_DIR}/web/index.html
        ${PROJECT_SOURCE_DIR}/web/main.js)

set(NES_WEB_SHELL_OUTPUTS "")
foreach(shell_file ${NES_WEB_SHELL_FILES})
    get_filename_component(shell_name ${shell_file} NAME)
    add_custom_command(
            OUTPUT ${NES_WEB_OUTPUT_DIR}/${shell_name}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${shell_file} ${NES_WEB_OUTPUT_DIR}/${shell_name}
            DEPENDS ${shell_file})
    list(APPEND NES_WEB_SHELL_OUTPUTS ${NES_WEB_OUTPUT_DIR}/${shell_name})
endforeach()

add_custom_target(nes_web_shell DEPENDS ${NES_WEB_SHELL_OUTPUTS})
add_dependencies(nes_emulator_wasm nes_web_shell)
