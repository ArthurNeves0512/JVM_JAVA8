# JVM Java 8 — Class File Reader

Reads and prints the structure of a compiled Java `.class` file.

## Build

```bash
make
```

## Run

```bash
# output saved to <file>.txt by default
./main path/to/File.class

# specify output file
./main path/to/File.class -o output.txt

# also print to terminal
./main path/to/File.class --print
./main path/to/File.class -o output.txt --print
```

## Tests

```bash
make test
```
