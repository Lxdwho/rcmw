# ROOT

load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "test_hello_pub",
    srcs = ["test_hello_pub.cpp"],
    deps = ["//:init",
            "//node:node",
            "//serialize:serialize",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "test_hello_sub",
    srcs = ["test_hello_sub.cpp"],
    deps = ["//:init",
            "//node:node",
            "//serialize:serialize",
    ],
    visibility = ["//visibility:public"],
)
