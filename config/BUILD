# Copyright 2025 rcmw project
#
# Blocker utilities for the rcmw middleware.
# Includes: 

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "config",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    deps = ["//serialize:serialize",
            "//logger:logger",
            "//thirdparty:nlohmann_json",
    ],
    visibility = ["//visibility:public"],
)
