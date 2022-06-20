package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "core",
    srcs = [
        "abstract_arena.cc",
        "abstract_controller.cc",
        "abstract_game_area.cc",
    ],
    hdrs = [
        "abstract_arena.h",
        "abstract_controller.h",
        "abstract_game_area.h",
    ],
    deps = [
        "//exploratron/core/utils:maths",
        "//exploratron/core/utils:register",
        "//exploratron/core/utils:terminal",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "evaluate",
    srcs = ["evaluate.cc"],
    hdrs = ["evaluate.h"],
    deps = [
        ":core",
        "//exploratron/core/utils:logging",
        "//exploratron/core/utils:terminal",
    ],
)
