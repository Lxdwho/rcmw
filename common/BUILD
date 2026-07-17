# Copyright 2025 rcmw project
#
# Blocker utilities for the rcmw middleware.
# Includes: 

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "common",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    deps = ["//config:config",
            "//base:base",
            "//thirdparty:nlohmann_json",
            "//logger:logger"
    ],
    visibility = ["//visibility:public"],
)
