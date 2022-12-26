workspace(name = "learning_behavior")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Skylib
http_archive(
    name = "bazel_skylib",
    sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
    ],
)
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()

# Cpp
http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-master",
    urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
)

# Emscripten
http_archive(
    name = "emsdk",
    # 3.1.28 3.1.18
    # strip_prefix = "emsdk-main/bazel",
    # url = "https://github.com/emscripten-core/emsdk/archive/refs/heads/main.zip",
    strip_prefix = "emsdk-3.1.28/bazel",
    url = "https://github.com/emscripten-core/emsdk/archive/refs/tags/3.1.28.zip",
    #sha256 = "54ce0eec2e6cc47cab9f12de2adf34e2ee1a12ff780ce06f10eb45d54f18b3f4",
)
load("@emsdk//:deps.bzl", emsdk_deps = "deps")
emsdk_deps()
load("@emsdk//:emscripten_deps.bzl", emsdk_emscripten_deps = "emscripten_deps")
emsdk_emscripten_deps()
