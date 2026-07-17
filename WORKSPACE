# rcmw Begin   !!!废弃不用了
workspace(name = "rcmw")

# 导入 http_archive 工具
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# http_archive(
#     name = "",            # 下载的外部依赖的名称
#     sha256 = "",          # 下载文件的sha265校验值，防止文件变化
#     strip_prefix = "",    # 压缩包解压后的顶级目录名称
#     url = "",             # 下载地址
# )

# ==============================================
# 下载 rules_foreign_cc 用于辅助bazel编译Cmake项目
# ==============================================
http_archive(
    name = "rules_foreign_cc",
    sha256 = "",
    strip_prefix = "rules_foreign_cc-0.12.0",
    url = "https://github.com/bazelbuild/rules_foreign_cc/releases/download/0.12.0/rules_foreign_cc-0.12.0.tar.gz",
)
load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")
rules_foreign_cc_dependencies()

# ==============================================
# GoogleTest
# ==============================================
http_archive(
    name = "com_google_googletest",
    sha256 = "",
    strip_prefix = "googletest-1.15.2",
    urls = ["https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz"],
)

# ==============================================
# nlohman/json
# ==============================================
http_archive(
    name = "json",
    build_file_content = """
cc_library(
    name = "nlohmann_json",
    hdrs = glob(["include/**/*.hpp"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
    sha256 = "",
    strip_prefix = "json-3.11.3",
    urls = ["https://github.com/nlohmann/json/releases/download/v3.11.3/json-3.11.3.tar.xz"],
)

# ============================================================
# FastCDR (FastDDS 的依赖)
# ============================================================
http_archive(
    name = "fastcdr",
    build_file_content = """
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

cmake(
    name = "fastcdr",
    lib_source = glob(["**"]),
    visibility = ["//visibility:public"],
)
""",
    sha256 = "",
    strip_prefix = "Fast-CDR-1.1.0",
    urls = ["https://github.com/eProsima/Fast-CDR/archive/refs/tags/v1.1.0.tar.gz"],
)

# ============================================================
# FastDDS
# ============================================================
http_archive(
    name = "fastrtps",
    build_file_content = """
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

cmake(
    name = "fastrtps",
    lib_source = glob(["**"]),
    deps = ["@fastcdr//:fastcdr"],
    visibility = ["//visibility:public"],
)
""",
    sha256 = "",
    strip_prefix = "Fast-DDS-2.3.0",
    urls = ["https://github.com/eProsima/Fast-DDS/archive/refs/tags/v2.3.0.tar.gz"],
)
