const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const copyFiles = b.addWriteFiles();
    copyFiles.addCopyFileToSource(.{ .path = "src/config.h.generic" }, "src/config.h");
    copyFiles.addCopyFileToSource(.{ .path = "src/pcre2.h.generic" }, "src/pcre2.h");
    copyFiles.addCopyFileToSource(.{ .path = "src/pcre2_chartables.c.dist" }, "src/pcre2_chartables.c");

    const lib = b.addStaticLibrary(.{
        .name = "pcre2",
        .target = target,
        .optimize = optimize,
    });
    lib.addIncludePath(.{ .path = "src" });
    lib.addCSourceFiles(&.{
        "src/pcre2_auto_possess.c",
        "src/pcre2_chkdint.c",
        "src/pcre2_compile.c",
        "src/pcre2_config.c",
        "src/pcre2_context.c",
        "src/pcre2_convert.c",
        "src/pcre2_dfa_match.c",
        "src/pcre2_error.c",
        "src/pcre2_extuni.c",
        "src/pcre2_find_bracket.c",
        "src/pcre2_maketables.c",
        "src/pcre2_match.c",
        "src/pcre2_match_data.c",
        "src/pcre2_newline.c",
        "src/pcre2_ord2utf.c",
        "src/pcre2_pattern_info.c",
        "src/pcre2_script_run.c",
        "src/pcre2_serialize.c",
        "src/pcre2_string_utils.c",
        "src/pcre2_study.c",
        "src/pcre2_substitute.c",
        "src/pcre2_substring.c",
        "src/pcre2_tables.c",
        "src/pcre2_ucd.c",
        "src/pcre2_valid_utf.c",
        "src/pcre2_xclass.c",
        "src/pcre2_chartables.c",
    }, &.{
        "-std=c99",
        "-DHAVE_CONFIG_H",
        "-DPCRE2_CODE_UNIT_WIDTH=8",
        "-DPCRE2_STATIC",
    });
    lib.step.dependOn(&copyFiles.step);
    lib.installHeader("src/pcre2.h.generic", "pcre2.h");
    lib.linkLibC();
    b.installArtifact(lib);
}
