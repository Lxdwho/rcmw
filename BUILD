# ROOT

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "init",
    srcs = ["init.cpp"],
    hdrs = ["init.h"],
    deps = ["//logger:logger",
            "//:state",
            "//node:node",
            "//common:common",
            "//scheduler:scheduler",
            "//transport:transport",
            "//discovery:discovery",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "state",
    srcs = ["state.cpp"],
    hdrs = ["state.h"],
    deps = ["//logger:logger",],
    visibility = ["//visibility:public"],
)
