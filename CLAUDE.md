# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project does

A C11 tool that reads and prints the structure of a compiled Java `.class` file — the magic number, version, constant pool, access flags, interfaces, fields, methods, and Code attributes.

## Commands

```bash
make                          # build → ./main
make run                      # build and run on data/examples/Opa.class (default)
make run FILE=path/to/X.class # build and run on a specific file
make test                     # build and run all tests
make clean                    # remove build/ and ./main

./main path/to/File.class             # output to <file>.txt by default
./main path/to/File.class -o out.txt  # explicit output file
./main path/to/File.class --print     # also echo to terminal
```

Run a single test binary directly after `make test` has built it:

```bash
./build/tests/test_alloc_class_file
./build/tests/test_read_byte
./build/tests/test_printer
./build/tests/test_fields_interfaces
./build/tests/test_methods
```

## Architecture

### Data flow in `main`

```
parse_args → readFile → classFilesSetup → readInterfaces → readFields
           → readMethodsCount → readMethods → printClassFile → printMethods
```

Output is redirected to a file via `freopen`; `--print` uses `dup(STDOUT_FILENO)` before the redirect and replays the file back to the real terminal afterwards.

### Module layout

| Path | Role |
|------|------|
| `src/lib/types/dataTypes.h` | `u1/u2/u4/u8` typedefs mapping to `uint8_t`…`uint64_t` |
| `src/lib/types/consts.h` | `CONSTANT_POOL_TAGS` and `ACCESS_MODIFIERS` enums |
| `src/lib/types/class_file/cp_info.h` | One struct per CP entry kind (`CONSTANT_Class_info`, etc.) |
| `src/lib/types/constant_pool.h` | `cp_info` — a tagged union over all CP entry structs |
| `src/lib/types/class_file/dot_class.h` | `ClassFile` — the top-level struct |
| `src/lib/types/class_file/methods_info.h` | `field_info` and `method_info` structs |
| `src/lib/types/class_file/attributes_info.h` | `attribute_info` struct |
| `src/lib/types/attribute.h/c` | `Code_attribute`, `readAttribute`, `readCodeAttribute`, `printCodeAttribute`, `getUtf8` |
| `src/lib/file/read_byte.h` | Inline big-endian readers: `u1Read`, `u2Read`, `u4Read` |
| `src/lib/file/read_file.h/c` | `readFile()` — opens the `.class` file |
| `src/lib/class_loader/loader.h/c` | `classFilesSetup()` — parses header and constant pool |
| `src/lib/class_loader/fields_interfaces.h` | `readInterfaces()`, `readFields()` — header-only implementations |
| `src/lib/class_loader/methods.h` | `readMethodsCount()`, `readMethods()` — header-only implementation |
| `src/lib/printer/printer.h/c` | `printClassFile()`, `printMethods()`, `printFileToTerminal()` |
| `src/lib/utils/args.h/c` | CLI argument parsing into `Args` struct |

### Important implementation details

- **Header-only modules**: `fields_interfaces.h` and `methods.h` contain full function *definitions* (not just declarations). They are `#include`d only from `src/main.c`, so there is no ODR violation, but adding a second translation unit that includes them would cause duplicate symbol errors.

- **Constant pool indexing**: The JVM spec says CP indices are 1-based. The array is allocated at size `constant_pool_count` and index 0 is never written. `Long` and `Double` entries consume two consecutive slots; the phantom slot gets `tag = -1`.

- **Attribute dispatch**: `readAttribute` peeks at the name string to decide whether to parse a `Code_attribute` (storing a `Code_attribute*` cast to `u1*` in `attr.info`) or to read raw bytes. Callers that use a `Code` attribute must cast `attr.info` back to `Code_attribute*`.

- **Test structure**: Each file in `tests/` is a self-contained program with its own `main`. They use `assert()` and the `RUN_TEST` macro; a failed assert causes a non-zero exit, which `make test` treats as failure.

- **Include paths**: The Makefile adds `-Isrc` and several sub-directories, so headers are included as `"lib/types/class_file/dot_class.h"` (relative to `src/`).
