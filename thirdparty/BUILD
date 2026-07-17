# ============================================================
# 第三方引入库
# ============================================================
load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "gtest",
    srcs = ["googletest/lib/libgtest.a"],
    hdrs = glob(["googletest/include/**/*.h"]),
    includes = ["googletest/include"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "gtest_main",
    srcs = ["googletest/lib/libgtest_main.a"],
    hdrs = glob(["googletest/include/**/*.h"]),
    includes = ["googletest/include"],
    deps = [":gtest"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "nlohmann_json",
    hdrs = glob(["nlohmann/**/*.hpp"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "fastcdr",
    srcs = glob(["Fast-DDS/lib/libfastcdr.so*"]),
    hdrs = glob(["Fast-DDS/include/**/*.h", "Fast-DDS/include/**/*.hpp"]),
    includes = ["Fast-DDS/include"],
    linkstatic = False,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "fastrtps",
    srcs = glob(["Fast-DDS/lib/libfastrtps.so*"]),
    hdrs = glob(["Fast-DDS/include/**"]),
    includes = ["Fast-DDS/include"],
    linkstatic = False,
    deps = [":fastcdr"],
    visibility = ["//visibility:public"],
)
