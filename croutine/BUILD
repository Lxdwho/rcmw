# Copyright 2025 rcmw project
#
# Blocker utilities for the rcmw middleware.
# Includes: 

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "croutine",
    srcs = glob(["*.cpp", "*.S"]),
    hdrs = glob(["*.h"]),
    deps = ["//logger:logger",
            "//common:common",
            "//data:data",
            "//data:data_visitor",
            "//event:event",
    ],
    visibility = ["//visibility:public"],
)
