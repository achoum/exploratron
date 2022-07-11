package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "external_optimizer",
    srcs = ["external_optimizer.cc"],
    hdrs = ["external_optimizer.h"],
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
    data = [
        ":optimizer_pgpelib.py",
    ],
    linkopts = [
        "-pthread",
        "-ltbb",
    ],
    deps = [
        ":external_optimizer",
        "//exploratron/arena:all_arenas",
        "//exploratron/core",
        "//exploratron/core:evaluate",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)
