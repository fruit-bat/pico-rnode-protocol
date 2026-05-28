# pico-rnode-protocol
RNODE protocol layer

Inspired by, but not officially part of, [Reticulum](https://github.com/markqvist/Reticulum);
thanks to [markqvist](https://github.com/markqvist) for a very interesting project.

This library is part of an attempt to split the Reticulum protocol into testable, replaceable layers, written in C.
It is intended to be suitable for use on microcontrollers, and where possible, 
avoids allocations and external dependencies. 

This library is a work in progress.

Also in the suite:
* [pico-kiss-protocol](https://github.com/fruit-bat/pico-kiss-protocol)
* [pico-rnode-protocol](https://github.com/fruit-bat/pico-rnode-protocol)

## Host unit tests
A simple host test target is available under `tests/`.

Build and run from the repository root:

```bash
cd tests
cmake -S . -B build
cmake --build build && ctest --verbose --test-dir build
```

or for just specific tests
```bash
cd tests
cmake -S . -B build
cmake --build build && ctest --verbose --test-dir . -R host_tests_round_trip
```

This compiles `pico-rnode-protocol` as a normal host executable and runs the unit tests without Pico hardware.
