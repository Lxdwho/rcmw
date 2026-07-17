# Copyright 2025 rcmw project
#
# Blocker utilities for the rcmw middleware.
# Includes: 

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "node",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    deps = ["//logger:logger",
            "//event:event",
            "//common:common",
            "//config:config",
            "//data:data",
            "//transport:receiver",
            "//transport:transmitter",
            "//transport:transport",
            "//thirdparty:fastrtps",
            "//blocker:blocker_manager",
            "//croutine:croutine",
            "//scheduler:scheduler",
            "//discovery:discovery",
            "//discovery:specific_manager",
    ],
    visibility = ["//visibility:public"],
)
