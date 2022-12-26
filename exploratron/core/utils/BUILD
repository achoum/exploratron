package(
    default_visibility=["//visibility:public"],
)

cc_library(
    name="logging",
    hdrs=["logging.h"],
)

cc_library(
    name="symbols",
    hdrs=["symbols.h"],
    srcs=["symbols.cc"],
    deps=[":logging"],
)


cc_library(
    name="terminal_interface",
    hdrs=["terminal_interface.h"],
    srcs=["terminal_interface.cc"],
    deps=[":logging",":symbols"],
)

cc_library(
    name="terminal_wasm",
    srcs=["terminal_wasm.cc"],
    hdrs=["terminal_wasm.h"],
    deps=[":logging", ":terminal_interface",":symbols"],
)

cc_library(
    name="terminal_sdl",
    srcs=["terminal_sdl.cc"],
    hdrs=["terminal_sdl.h"],
    deps=[":logging", ":terminal_interface","@com_google_absl//absl/strings",],
    linkopts=["-lSDL2", "-lSDL2_image", "-lSDL2_ttf"],
)

cc_library(
    name="terminal_sdl_wasm",
    srcs=["terminal_sdl_wasm.cc"],
    hdrs=["terminal_sdl_wasm.h"],
    deps=[":logging", ":terminal_interface" ,],
    copts=[
        #"-s ALLOW_MEMORY_GROWTH=1",
        #"-s EXIT_RUNTIME=0",
        #"-s FILESYSTEM=1",
        #"-s ASYNCIFY=1",
        # "--bind",
        #"--embed-file exploratron/assets@exploratron/assets/",
        "-s USE_SDL=2",
        "-s USE_SDL_TTF=2",
        "-s USE_SDL_IMAGE=2",
        #"-s SDL2_IMAGE_FORMATS='[\"png\"]'"
    ]
)

cc_library(
    name="terminal_linux",
    srcs=["terminal_linux.cc"],
    hdrs=["terminal_linux.h"],
    deps=[":logging", ":terminal_interface",":symbols"],
)

cc_library(
    name="terminal_win",
    srcs=["terminal_win.cc"],
    hdrs=["terminal_win.h"],
    deps=[":logging", ":terminal_interface"],
)

cc_library(
    name="terminal_ncurses",
    srcs=["terminal_ncurses.cc"],
    hdrs=["terminal_ncurses.h"],
    deps=[":logging", ":terminal_interface"],
    linkopts=["-lcurses"],
)


cc_library(
    name="terminal",
    srcs=["terminal.cc"],
    hdrs=["terminal.h"],
    defines=select({
        "//exploratron:terminal_windows_console": ["TERMINAL_WINDOWS_CONSOLE"],
        "//exploratron:terminal_linux_console": ["TERMINAL_LINUX_CONSOLE"],
        "//exploratron:terminal_ncuses_console": ["TERMINAL_NCURSES_CONSOLE"],
        "//exploratron:terminal_wasm_console": ["TERMINAL_WASM_CONSOLE"],
        "//exploratron:terminal_sdl_console": ["TERMINAL_SDL_CONSOLE"],
        "//exploratron:terminal_sdl_wasm_console": ["TERMINAL_SDL_WASM_CONSOLE"],
        "//conditions:default": [],
    }),
    deps=[
        ":logging", ":terminal_interface",
    ] + select({
        "//exploratron:terminal_windows_console": [":terminal_win"],
        "//exploratron:terminal_linux_console": [":terminal_linux"],
        "//exploratron:terminal_ncuses_console": [":terminal_ncurses"],
        "//exploratron:terminal_wasm_console": [":terminal_wasm"],
        "//exploratron:terminal_sdl_console": [":terminal_sdl"],
        "//exploratron:terminal_sdl_wasm_console": [":terminal_sdl_wasm"],
        "//conditions:default": [],
    }),
    #includes=["/home/achoum/tmp/bazel/external/emscripten_bin_linux/emscripten/cache/sysroot/include"],
    #includes=["/",],
    copts= select({
        "//exploratron:terminal_sdl_wasm_console": [
            #"-s USE_SDL=2",
            #"-s USE_SDL_TTF=2",
            #"-s USE_SDL_IMAGE=2",
            #"-s SDL2_IMAGE_FORMATS='[\"png\"]'",
            ],
        "//conditions:default": [],
    })
)

cc_library(
    name="maths",
    srcs=["maths.cc"],
    hdrs=["maths.h"],
    deps=[
        ":logging",
        ":macros",
    ],
)

cc_library(
    name="macros",
    hdrs=["macros.h"],
)

cc_library(
    name="neural_net",
    srcs=["neural_net.cc"],
    hdrs=["neural_net.h"],
    deps=[
        ":logging",
        ":macros",
        ":maths",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name="register",
    hdrs=["register.h"],
    deps=[":logging"],
)
