package(default_visibility = ["//visibility:public"])

genrule(
    name = "pack_game",
    srcs = [
        ":data",
        "//exploratron/cli:game_wasm",
    ],
    outs = ["compiled_web_game"],
    cmd = "IN=exploratron/web && " +
          "OUT=$(RULEDIR)/compiled_web_game && " +
          "mkdir -p $${OUT}/www/wasm && " +
          "cp -R $${IN}/start_server.sh $${OUT}/ && " +
          "cp -R $${IN}/www $${OUT}/ && " +
          "cp $(locations //exploratron/cli:game_wasm) $${OUT}/www/wasm/",
)

filegroup(
    name = "data",
    srcs = glob(["**/*"]),
)
