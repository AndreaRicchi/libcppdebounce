

# libcppdebounce [![Continuous Integration](https://github.com/AndreaRicchi/libcppdebounce/actions/workflows/ci.yml/badge.svg)](https://github.com/AndreaRicchi/libcppdebounce/actions/workflows/ci.yml)

Header-only C++17 library for safe **throttling** and **debouncing** using only the standard library.

`libcppdebounce` provides two lightweight utilities:

| Feature | Behavior |
|--------|----------|
| **Throttle** | First call is executed immediately, all subsequent calls are ignored for a fixed time interval |
| **Debounce** | Execution is deferred until calls stop; repeated calls reset the delay |

Useful for rate-limiting high-frequency events such as UI callbacks, hardware interrupts, logging bursts, periodic updates, and asynchronous input streams.

---

## Features

- 🟢 Header-only — just include and use
- 🔒 Thread-safe implementation
- ⏳ Based on `std::chrono` and `std::thread`
- 🚫 No external dependencies (Standard Library only)
- ⚙ Works on embedded, desktop and server environments
- 💡 Clean and minimal API

---

## Usage

```CMake
include(FetchContent)
FetchContent_Declare(
	cppdebounce
	GIT_REPOSITORY https://github.com/AndreaRicchi/libcppdebounce.git
	GIT_TAG vMAJOR.MINOR.PATCH
	FIND_PACKAGE_ARGS MAJOR.MINOR CONFIG
	)
FetchContent_MakeAvailable(cppdebounce)

target_link_libraries(<target> <PRIVATE|PUBLIC|INTERFACE> cppdebounce::cppdebounce)
```
