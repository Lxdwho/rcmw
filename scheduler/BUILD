# Copyright 2025 rcmw project
#
# Blocker utilities for the rcmw middleware.
# Includes: 

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "common",
    srcs = glob(["common/*.cpp"]),
    hdrs = glob(["common/*.h"]),
    deps = ["//logger:logger",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "scheduler",
    srcs = glob(["policy/*.cpp", "*.cpp"]),
    hdrs = glob(["policy/*.h", "*.h"]),
    deps = ["//logger:logger",
            "//base:base",
            "//config:config",
            "//common:common",
            "//croutine:croutine",
            "//scheduler:common",
            "//time:time",
            "//thirdparty:fastrtps",
    ],
    visibility = ["//visibility:public"],
)
