package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "hill_climbing",
    srcs = ["hill_climbing.cc"],
    hdrs = ["hill_climbing.h"],
    deps = [
        "//exploratron/core",
        "//exploratron/core/utils:logging",
        "//exploratron/core/utils:macros",
        "//exploratron/core/utils:neural_net",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "train_main",
    srcs = ["train_main.cc"],
    deps = [
        ":hill_climbing",
        "//exploratron/arena:all_arenas",
        "//exploratron/core",
        "//exploratron/core:evaluate",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)
