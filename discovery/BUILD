# Copyright 2025 rcmw project
#
# Blocker utilities for the rcmw middleware.
# Includes: 

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "communication",
    srcs = glob(["communication/*.cpp"]),
    hdrs = glob(["communication/*.h"]),
    deps = ["//logger:logger",
            "//transport:message",
            "//thirdparty:fastrtps",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "role",
    srcs = glob(["role/*.cpp"]),
    hdrs = glob(["role/*.h"]),
    deps = ["//config:config",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "container",
    srcs = glob(["container/*.cpp"]),
    hdrs = glob(["container/*.h"]),
    deps = ["//logger:logger",
            "//discovery:role",
            "//base:base",
            "//transport:message",
            "//thirdparty:fastrtps",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "specific_manager",
    srcs = glob(["specific_manager/*.cpp"]),
    hdrs = glob(["specific_manager/*.h"]),
    deps = ["//logger:logger",
            "//base:base",
            "//config:config",
            "//common:common",
            "//discovery:role",
            "//discovery:container",
            "//discovery:communication",
            "//transport:qos",
            "//transport:rtps",
            "//time:time",
            "//thirdparty:fastrtps",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "discovery",
    srcs = ["topology_manager.cpp"],
    hdrs = ["topology_manager.h"],
    deps = ["//logger:logger",
            "//base:base",
            "//config:config",
            "//common:common",
            "//discovery:communication",
            "//discovery:specific_manager",
            "//transport:rtps",
            "//time:time",
            "//thirdparty:fastrtps",
    ],
    visibility = ["//visibility:public"],
)
