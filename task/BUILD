# ROOT

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "task",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    deps = ["//croutine:croutine",
            "//scheduler:scheduler",
            "//base:base",
            "//common:common",
    ],
    visibility = ["//visibility:public"],
)