package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common_game",
    srcs = ["common_game.cc"],
    hdrs = ["common_game.h"],
    deps = [
        "//exploratron/core",
        "//exploratron/core/utils:logging",
        "//exploratron/core/utils:terminal",
        "//third_party/lodepng",
        "//third_party/tinyxml2",
        "@com_google_absl//absl/strings",
    ],
    alwayslink = 1,
)
