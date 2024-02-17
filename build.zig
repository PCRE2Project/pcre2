const std = @import("std");

pub const CodeUnitWidth = enum {
    @"8",
    @"16",
    @"32",
};

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const linkage = b.option(std.Build.Step.Compile.Linkage, "linkage", "whether to statically or dynamically link the library") orelse .static;
    const codeUnitWidth = b.option(CodeUnitWidth, "code-unit-width", "Sets the code unit width") orelse .@"8";

    const copyFiles = b.addWriteFiles();
    _ = copyFiles.addCopyFile(.{ .path = "src/config.h.generic" }, "config.h");
    _ = copyFiles.addCopyFile(.{ .path = "src/pcre2.h.generic" }, "pcre2.h");

    const lib = std.Build.Step.Compile.create(b, .{
        .name = b.fmt("pcre2-{s}", .{@tagName(codeUnitWidth)}),
        .root_module = .{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        },
        .kind = .lib,
        .linkage = linkage,
    });

    if (linkage == .static) {
        try lib.root_module.c_macros.append(b.allocator, "-DPCRE2_STATIC");
    }

    lib.root_module.addCMacro("PCRE2_CODE_UNIT_WIDTH", @tagName(codeUnitWidth));

    lib.addCSourceFile(.{
        .file = copyFiles.addCopyFile(.{ .path = "src/pcre2_chartables.c.dist" }, "pcre2_chartables.c"),
        .flags = &.{
            "-DHAVE_CONFIG_H",
        },
    });

    lib.addIncludePath(.{ .path = b.pathFromRoot("src") });
    lib.addIncludePath(copyFiles.getDirectory());

    lib.addCSourceFiles(.{
        .files = &.{
            b.pathFromRoot("src/pcre2_auto_possess.c"),
            b.pathFromRoot("src/pcre2_chkdint.c"),
            b.pathFromRoot("src/pcre2_compile.c"),
            b.pathFromRoot("src/pcre2_config.c"),
            b.pathFromRoot("src/pcre2_context.c"),
            b.pathFromRoot("src/pcre2_convert.c"),
            b.pathFromRoot("src/pcre2_dfa_match.c"),
            b.pathFromRoot("src/pcre2_error.c"),
            b.pathFromRoot("src/pcre2_extuni.c"),
            b.pathFromRoot("src/pcre2_find_bracket.c"),
            b.pathFromRoot("src/pcre2_maketables.c"),
            b.pathFromRoot("src/pcre2_match.c"),
            b.pathFromRoot("src/pcre2_match_data.c"),
            b.pathFromRoot("src/pcre2_newline.c"),
            b.pathFromRoot("src/pcre2_ord2utf.c"),
            b.pathFromRoot("src/pcre2_pattern_info.c"),
            b.pathFromRoot("src/pcre2_script_run.c"),
            b.pathFromRoot("src/pcre2_serialize.c"),
            b.pathFromRoot("src/pcre2_string_utils.c"),
            b.pathFromRoot("src/pcre2_study.c"),
            b.pathFromRoot("src/pcre2_substitute.c"),
            b.pathFromRoot("src/pcre2_substring.c"),
            b.pathFromRoot("src/pcre2_tables.c"),
            b.pathFromRoot("src/pcre2_ucd.c"),
            b.pathFromRoot("src/pcre2_valid_utf.c"),
            b.pathFromRoot("src/pcre2_xclass.c"),
        },
        .flags = &.{
            "-DHAVE_CONFIG_H",
            "-DPCRE2_CODE_UNIT_WIDTH=8",
            "-DPCRE2_STATIC",
        },
    });

    lib.installHeader("src/pcre2.h.generic", "pcre2.h");
    b.installArtifact(lib);
}
