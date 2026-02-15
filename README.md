# cx_tests

Test suite for the [cx library](https://github.com/toddvernon/cx).

## Overview

cx_tests provides unit tests for cx library modules. Each test directory corresponds to a cx module and contains standalone test programs that exercise the module's functionality.

## Directory Structure

cx_tests expects this layout:

```
cx/
├── cx/              <- cx library
├── cx_tests/        <- this repo
├── cx_apps/
│   └── cm/          <- CMacs terminal editor
└── lib/             <- built libraries (created by cx make)
```

## Getting Started

```bash
# Ensure cx is built first
cd ~/dev/cx/cx
make

# Build all tests
cd ~/dev/cx/cx_tests
make

# Run all tests
make test
```

### Running Individual Tests

Each test directory contains its own makefile:

```bash
cd cxstring
make
make test
```

## Test Modules

| Directory | Tests |
|-----------|-------|
| cxb64 | Base64 encoding/decoding |
| cxbuildoutput | Build output parsing |
| cxcallback | Callback/functor system |
| cxcolorize | Syntax colorization |
| cxcommandcompleter | Tab-completion engine |
| cxdatetime | Date/time handling |
| cxdirectory | Directory operations |
| cxeditbuffer | Text editing buffer |
| cxeditline | Line editing |
| cxexpression | Expression evaluation |
| cxfile | File I/O |
| cxfunctor | Function objects |
| cxhandle | Handle/reference system |
| cxhashmap | Hash map container |
| cxjson | JSON parsing |
| cxlog | Logging |
| cxlogfile | Log file handling |
| cxnet | Network sockets |
| cxprop | Properties/configuration |
| cxregex | Regular expressions (Linux/macOS) |
| cxscreen | Terminal/screen handling |
| cxsheetmodel | Spreadsheet model |
| cxslist | Linked list container |
| cxstar | Star/wildcard matching |
| cxstring | String operations |
| cxthread | Threading (Linux/macOS) |
| cxtime | Time utilities |
| cxtz | Timezone handling |
| cxutfcharacter | UTF-8 character handling |
| cxutfeditbuffer | UTF-8 edit buffer |
| cxutfstring | UTF-8 string operations |
| cxutfstringlist | UTF-8 string list |
| keyboard | Keyboard input |

## Requirements

- cx library must be built first
- GNU Make
- GCC 2.8.1 or later (for vintage systems)

Both are available at [gnu.org](https://www.gnu.org/software/).

## Supported Platforms

- macOS (ARM64 and x86_64)
- Linux (x86_64)
- Solaris / SunOS
- IRIX 6.5
- NetBSD

## Related Projects

- [cx](https://github.com/toddvernon/cx) — The cx library
- [CMacs](https://github.com/toddvernon/cm) — Terminal text editor built on cx

## License

Apache License 2.0. See LICENSE file.
