const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const stdin_file = std.fs.File.stdin();
    const stdout_file = std.fs.File.stdout();

    var read_buf: [4096]u8 = undefined;
    var write_buf: [4096]u8 = undefined;

    var file_reader = stdin_file.readerStreaming(&read_buf);
    var file_writer = stdout_file.writerStreaming(&write_buf);

    while (true) {
        const msg = readMessage(allocator, &file_reader.interface) catch |err| switch (err) {
            error.EndOfStream => break,
            else => return err,
        };
        defer allocator.free(msg);

        const keep_running = try handleMessage(allocator, &file_writer.interface, msg);
        try file_writer.interface.flush();

        if (!keep_running) break;
    }
}

/// Read one LSP message. Caller owns the returned slice.
fn readMessage(allocator: std.mem.Allocator, reader: anytype) ![]u8 {
    var content_length: usize = 0;

    // Parse headers until blank line.
    while (true) {
        // takeDelimiterInclusive returns the line including the \n delimiter.
        const line_with_delim = reader.takeDelimiterInclusive('\n') catch |err| switch (err) {
            error.EndOfStream => return error.EndOfStream,
            else => return err,
        };
        // Strip trailing \r\n or just \n.
        var line = line_with_delim;
        if (line.len > 0 and line[line.len - 1] == '\n') line = line[0 .. line.len - 1];
        if (line.len > 0 and line[line.len - 1] == '\r') line = line[0 .. line.len - 1];

        if (line.len == 0) break;

        if (std.ascii.startsWithIgnoreCase(line, "content-length:")) {
            const val = std.mem.trimLeft(u8, line["content-length:".len..], " \t");
            content_length = try std.fmt.parseInt(usize, val, 10);
        }
    }

    if (content_length == 0) return error.EndOfStream;

    return reader.readAlloc(allocator, content_length);
}

/// Handle one JSON-RPC message. Returns false when the server should exit.
fn handleMessage(allocator: std.mem.Allocator, writer: anytype, msg: []const u8) !bool {
    const parsed = try std.json.parseFromSlice(std.json.Value, allocator, msg, .{});
    defer parsed.deinit();

    const root = parsed.value;
    if (root != .object) return true;

    const method_val = root.object.get("method") orelse return true;
    if (method_val != .string) return true;
    const method = method_val.string;

    if (std.mem.eql(u8, method, "initialize")) {
        const id_val = root.object.get("id") orelse return true;
        try sendInitializeResult(allocator, writer, id_val);
    } else if (std.mem.eql(u8, method, "shutdown")) {
        const id_val = root.object.get("id") orelse return true;
        try sendNullResult(allocator, writer, id_val);
    } else if (std.mem.eql(u8, method, "exit")) {
        return false;
    }
    // All other methods (notifications like "initialized") are silently ignored.

    return true;
}

fn sendInitializeResult(allocator: std.mem.Allocator, writer: anytype, id: std.json.Value) !void {
    const id_str = try jsonIdToString(allocator, id);
    defer allocator.free(id_str);

    const body = try std.fmt.allocPrint(allocator,
        \\{{"jsonrpc":"2.0","id":{s},"result":{{"capabilities":{{}}}}}}
    , .{id_str});
    defer allocator.free(body);

    try writeFrame(writer, body);
}

fn sendNullResult(allocator: std.mem.Allocator, writer: anytype, id: std.json.Value) !void {
    const id_str = try jsonIdToString(allocator, id);
    defer allocator.free(id_str);

    const body = try std.fmt.allocPrint(allocator,
        \\{{"jsonrpc":"2.0","id":{s},"result":null}}
    , .{id_str});
    defer allocator.free(body);

    try writeFrame(writer, body);
}

/// Serialize a JSON-RPC id (integer, string, or null) to its JSON text.
/// Caller owns the returned slice.
fn jsonIdToString(allocator: std.mem.Allocator, val: std.json.Value) ![]u8 {
    return switch (val) {
        .integer => |n| std.fmt.allocPrint(allocator, "{d}", .{n}),
        .string => |s| std.fmt.allocPrint(allocator, "\"{s}\"", .{s}),
        .null => std.fmt.allocPrint(allocator, "null", .{}),
        else => std.fmt.allocPrint(allocator, "null", .{}),
    };
}

fn writeFrame(writer: anytype, body: []const u8) !void {
    try writer.print("Content-Length: {d}\r\n\r\n{s}", .{ body.len, body });
}
