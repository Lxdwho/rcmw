# Copyright 2025 rcmw project
#
# Blocker utilities for the rcmw middleware.
# Includes: 

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "buffer",
    hdrs = glob(["buffer/*.h"]),
    deps = ["//logger:logger",
            "//common:common",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "data_fusion",
    hdrs = glob(["data_fusion/*.h"]),
    deps = ["//logger:logger",
            "//common:common",
            "//data:buffer",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "data_visitor",
    hdrs = glob(["data_visitor/*.h"]),
    deps = ["//logger:logger",
            "//common:common",
            "//data:buffer",
            "//data:data_fusion",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "data",
    hdrs = glob(["*.h"]),
    deps = ["//logger:logger",
            "//common:common",
            "//time:time",
            "//base:base",
            "//data:buffer",
            "//:state",
    ],
    visibility = ["//visibility:public"],
)

