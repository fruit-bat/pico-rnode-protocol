# pico-rnode-protocol
RNODE protocol layer

## Host unit tests
A simple host test target is available under `tests/`.

Build and run from the repository root:

```bash
cd tests
cmake -S . -B build
cmake --build build && ctest --verbose --test-dir build
```

This compiles `pico-rnode-protocol` as a normal host executable and runs the unit tests without Pico hardware.
