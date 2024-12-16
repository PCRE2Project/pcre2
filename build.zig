const std = @import("std");

pub const CodeUnitWidth = enum {
    @"8",
    @"16",
    @"32",
};

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const linkage = b.option(std.builtin.LinkMode, "linkage", "whether to statically or dynamically link the library") orelse @as(std.builtin.LinkMode, if (target.result.isGnuLibC()) .dynamic else .static);
    const codeUnitWidth = b.option(CodeUnitWidth, "code-unit-width", "Sets the code unit width") orelse .@"8";

    const copyFiles = b.addWriteFiles();
    _ = copyFiles.addCopyFile(b.path("src/config.h.generic"), "config.h");
    _ = copyFiles.addCopyFile(b.path("src/pcre2.h.generic"), "pcre2.h");

    // pcre2-8/16/32.so

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

    lib.defineCMacro("HAVE_CONFIG_H", null);
    lib.defineCMacro("HAVE_MEMMOVE", null);
    lib.defineCMacro("PCRE2_CODE_UNIT_WIDTH", @tagName(codeUnitWidth));
    lib.defineCMacro("SUPPORT_UNICODE", null);
    if (linkage == .static) {
        lib.defineCMacro("PCRE2_STATIC", null);
    }

    lib.addIncludePath(b.path("src"));
    lib.addIncludePath(copyFiles.getDirectory());

    lib.addCSourceFile(.{
        .file = copyFiles.addCopyFile(b.path("src/pcre2_chartables.c.dist"), "pcre2_chartables.c"),
    });

    lib.addCSourceFiles(.{
        .files = &.{
            "src/pcre2_auto_possess.c",
            "src/pcre2_chkdint.c",
            "src/pcre2_compile.c",
            "src/pcre2_compile_class.c",
            "src/pcre2_config.c",
            "src/pcre2_context.c",
            "src/pcre2_convert.c",
            "src/pcre2_dfa_match.c",
            "src/pcre2_error.c",
            "src/pcre2_extuni.c",
            "src/pcre2_find_bracket.c",
            "src/pcre2_jit_compile.c",
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
        },
    });

    lib.installHeader(b.path("src/pcre2.h.generic"), "pcre2.h");
    b.installArtifact(lib);


    // pcre2test

    const pcre2test = b.addExecutable(.{
        .name = "pcre2test",
        .target = target,
        .optimize = optimize,
    });


    // pcre2-posix.so

    if (codeUnitWidth == CodeUnitWidth.@"8") {
        const posixLib = std.Build.Step.Compile.create(b, .{
            .name = "pcre2-posix",
            .root_module = .{
                .target = target,
                .optimize = optimize,
                .link_libc = true,
            },
            .kind = .lib,
            .linkage = linkage,
        });

        posixLib.defineCMacro("HAVE_CONFIG_H", null);
        posixLib.defineCMacro("HAVE_MEMMOVE", null);
        posixLib.defineCMacro("PCRE2_CODE_UNIT_WIDTH", @tagName(codeUnitWidth));
        posixLib.defineCMacro("SUPPORT_UNICODE", null);
        if (linkage == .static) {
            posixLib.defineCMacro("PCRE2_STATIC", null);
        }

        posixLib.addIncludePath(b.path("src"));
        posixLib.addIncludePath(copyFiles.getDirectory());

        posixLib.addCSourceFiles(.{
            .files = &.{
                "src/pcre2posix.c",
            },
        });

        b.installArtifact(posixLib);

        pcre2test.linkLibrary(posixLib);
    }


    // pcre2test (again)

    pcre2test.defineCMacro("HAVE_CONFIG_H", null);
    pcre2test.defineCMacro("HAVE_MEMMOVE", null);
    if (target.result.os.tag != .windows) {
        pcre2test.defineCMacro("HAVE_UNISTD_H", null);
    }
    pcre2test.defineCMacro("HAVE_STRERROR", null);
    pcre2test.defineCMacro("SUPPORT_UNICODE", null);
    pcre2test.defineCMacro(b.fmt("SUPPORT_PCRE2_{s}", .{@tagName(codeUnitWidth)}), null);
    if (linkage == .static) {
        pcre2test.defineCMacro("PCRE2_STATIC", null);
    }

    pcre2test.addIncludePath(b.path("src"));
    pcre2test.addIncludePath(copyFiles.getDirectory());

    pcre2test.addCSourceFile(.{
        .file = b.path("src/pcre2test.c"),
    });

    pcre2test.linkLibC();
    pcre2test.linkLibrary(lib);

    b.installArtifact(pcre2test);
}
