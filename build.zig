const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target_input = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const build_pcre2grep = b.option(bool, "build_pcre2grep", "") orelse false;

    var target = target_input;

    if (target_input.getOs().tag == .linux) {
        // Zig defaults to musl libc on linux, but we want glibc as pcre2grep,
        // pcre2test and perhaps other parts of pcre2 do not work with musl.
        // This silently changes the target to glibc, which can be a little
        // confusing if someone tries to explicitly target musl. It would be
        // better if we could differentiate between target_input being
        // explicitly set or if it has the defualt value.
        // TODO: Add a way to check if a target is the default target or
        //       explicitly set, if explicitly set, raise a warning and do not
        //       set it to glibc so that the user sees the error
        target.abi = .gnu;
    }

    var flags = std.ArrayList([]const u8).init(b.allocator);
    defer flags.deinit();
    flags.append("-std=c99") catch unreachable;
    flags.append("-DHAVE_CONFIG_H") catch unreachable;
    flags.append("-DPCRE2_CODE_UNIT_WIDTH=8") catch unreachable;
    flags.append("-DPCRE2_STATIC") catch unreachable;
    flags.append("-DMAX_VARLOOKBEHIND=255") catch unreachable;

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
    }, flags.items);
    lib.step.dependOn(&copyFiles.step);
    lib.installHeader("src/pcre2.h", "pcre2.h");
    lib.linkLibC();
    b.installArtifact(lib);

    if (build_pcre2grep) {
        const exe_pcre2grep = b.addExecutable(.{
            .name = "pcre2grep",
            .target = target,
            .optimize = optimize,
        });
        exe_pcre2grep.addIncludePath(.{ .path = "src" });
        exe_pcre2grep.addCSourceFiles(&.{
            "src/pcre2grep.c",
        }, flags.items);
        exe_pcre2grep.linkLibrary(lib);
        exe_pcre2grep.linkLibC();
        b.installArtifact(exe_pcre2grep);
    }
}
