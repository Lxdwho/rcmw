# Copyright 2025 rcmw project
#
# Blocker utilities for the rcmw middleware.
# Includes: 

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "common",
    srcs = glob(["common/*.cpp"]),
    hdrs = glob(["common/*.h"]),
    deps = ["//config:config",
            "//common:common"
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "qos",
    srcs = glob(["qos/*.cpp"]),
    hdrs = glob(["qos/*.h"]),
    deps = ["//config:config",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "message",
    srcs = glob(["message/*.cpp"]),
    hdrs = glob(["message/*.h"]),
    deps = ["//transport:common",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "shm",
    srcs = glob(["shm/*.cpp"]),
    hdrs = glob(["shm/*.h"]),
    deps = ["//logger:logger",
            "//common:common",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "rtps",
    srcs = glob(["rtps/*.cpp"]),
    hdrs = glob(["rtps/*.h"]),
    deps = ["//logger:logger",
            "//common:common",
            "//config:config",
            "//transport:qos",
            "//transport:message",
            "//thirdparty:fastrtps",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "dispatcher",
    srcs = glob(["dispatcher/*.cpp"]),
    hdrs = glob(["dispatcher/*.h"]),
    deps = ["//logger:logger",
            "//base:base",
            "//transport:shm",
            "//transport:rtps",
            "//common:common",
            "//config:config",
            "//serialize:serialize",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "receiver",
    hdrs = glob(["receiver/*.h"]),
    deps = ["//logger:logger",
            "//transport:dispatcher",
            "//transport:common",
            "//transport:message",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transmitter",
    hdrs = glob(["transmitter/*.h"]),
    deps = ["//time:time",
            "//event:event",
            "//logger:logger",
            "//config:config",
            "//serialize:serialize",
            "//transport:common",
            "//transport:message",
            "//transport:shm",
            "//transport:rtps",
            "//thirdparty:fastrtps",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport",
    srcs = ["transport.cpp"],
    hdrs = ["transport.h"],
    deps = ["//logger:logger",
            "//config:config",
            "//common:common",
            "//transport:transmitter",
            "//transport:receiver",
    ],
    visibility = ["//visibility:public"],
)
