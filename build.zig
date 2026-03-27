const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCSourceFile(.{ .file = b.path("src/main.c"), .flags = &.{ "-Wall", "-Wextra" } });

    const exe = b.addExecutable(.{
        .name = "anakins-c-ls",
        .root_module = mod,
    });

    b.installArtifact(exe);
}
