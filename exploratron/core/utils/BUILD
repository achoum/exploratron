package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "logging",
    hdrs = ["logging.h"],
)

cc_library(
    name = "terminal",
    srcs = ["terminal.cc"],
    hdrs = ["terminal.h"],
    defines = select({
        "//exploratron:terminal_windows_console": ["TERMINAL_WINDOWS_CONSOLE"],
        "//exploratron:terminal_linux_console": ["TERMINAL_LINUX_CONSOLE"],
        "//exploratron:terminal_linux_console_ncuses": ["TERMINAL_LINUX_CONSOLE_NCURSES"],
        "//exploratron:terminal_wasm_console": ["TERMINAL_WASM_CONSOLE"],
        "//conditions:default": [],
    }),
    linkopts = select({
        "//exploratron:terminal_linux_console_ncuses": ["-lcurses"],
        "//conditions:default": [],
    }),
    deps = [
        ":logging",
    ],
)

cc_library(
    name = "maths",
    srcs = ["maths.cc"],
    hdrs = ["maths.h"],
    deps = [
        ":logging",
        ":macros",
    ],
)

cc_library(
    name = "macros",
    hdrs = ["macros.h"],
)

cc_library(
    name = "neural_net",
    srcs = ["neural_net.cc"],
    hdrs = ["neural_net.h"],
    deps = [
        ":logging",
        ":macros",
        ":maths",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "register",
    hdrs = ["register.h"],
    deps = [":logging"],
)
