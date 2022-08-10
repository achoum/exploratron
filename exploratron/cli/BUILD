load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "evaluate_main",
    srcs = ["evaluate_main.cc"],
    deps = [
        "//exploratron/arena:all_arenas",
        "//exploratron/controller:all_controllers",
        "//exploratron/core",
        "//exploratron/core:evaluate",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)

cc_library(
    name = "game_lib",
    srcs = ["game.cc"],
    data = ["//exploratron/assets"],
    defines = select({
        "//exploratron:terminal_wasm_console": [
            "NO_QUIT",
        ],
        "//exploratron:terminal_linux_console": [
            "SKIP_INTRO",
            "SHOW_STACK_TRACE_IF_ABORT",
        ],
        "//conditions:default": ["SHOW_STACK_TRACE_IF_ABORT"],
    }),
    deps = [
        "//exploratron/arena:all_arenas",
        "//exploratron/arena/common_game",
        "//exploratron/controller:all_controllers",
        "//exploratron/controller/buffer",
        "//exploratron/core",
        "//exploratron/core:evaluate",
        "//exploratron/core/utils:terminal",
        "@com_google_absl//absl/debugging:failure_signal_handler",
        "@com_google_absl//absl/debugging:symbolize",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "game",
    deps = [":game_lib"],
)

# https://github.com/emscripten-core/emscripten/blob/main/src/settings.js
cc_binary(
    name = "game_wasm.js",
    data = ["//exploratron/assets"],
    linkopts = [
        "-s ALLOW_MEMORY_GROWTH=1",
        "-s EXIT_RUNTIME=0",
        "-s FILESYSTEM=1",
        "-s ASYNCIFY=1",
        "--bind",
        "--embed-file exploratron/assets@exploratron/assets/",
    ],
    deps = [":game_lib"],
)

wasm_cc_binary(
    name = "game_wasm",
    cc_target = ":game_wasm.js",
)
