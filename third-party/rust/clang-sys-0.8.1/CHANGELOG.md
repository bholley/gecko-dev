## [0.8.1] - 2016-7-20

### Changed
- Added `CLANG_PATH` environment variable for providing a path to `clang` executable
- Added usage of `llvm-config` to search for `clang`
- Added usage of `xcodebuild` to search for `clang` on OS X

## [0.8.0] - 2016-7-18

### Added
- Added support for `clang` 3.9.x

### Changed
- Bumped `libc` version to `0.2.14`

### Fixed
- Fixed `LIBCLANG_PATH` usage on Windows to search both the `bin` and `lib` directories
- Fixed search path parsing on OS X
- Fixed search path parsing on Windows
- Fixed default search path ordering on OS X

## [0.7.2] - 2016-6-17

### Fixed
- Fixed finding of `clang` executables when system has executables matching `clang-*`
  (e.g., `clang-format`)

## [0.7.1] - 2016-6-10

### Changed
- Bumped `libc` version to `0.2.12`

### Fixed
- Fixed finding of `clang` executables suffixed by their version (e.g., `clang-3.5`)

## [0.7.0] - 2016-5-31

### Changed
- Changed `Clang` struct `version` field type to `Option<CXVersion>`

## [0.6.0] - 2016-5-26

### Added
- Added `support` module

### Fixed
- Fixed `libclang` linking on FreeBSD
- Fixed `libclang` linking on Windows with the MSVC toolchain
- Improved `libclang` static linking

## [0.5.4] - 2016-5-19

### Changed
- Added implementations of `Default` for FFI structs

## [0.5.3] - 2016-5-17

### Changed
- Bumped `bitflags` version to `0.7.0`

## [0.5.2] - 2016-5-12

### Fixed
- Fixed `libclang` static linking

## [0.5.1] - 2016-5-10

### Fixed
- Fixed `libclang` linking on OS X
- Fixed `libclang` linking on Windows

## [0.5.0] - 2016-5-10

### Removed
- Removed `rustc_version` dependency
- Removed support for `LIBCLANG_STATIC` environment variable

### Changed
- Bumped `bitflags` version to `0.6.0`
- Bumped `libc` version to `0.2.11`
- Improved `libclang` search path
- Improved `libclang` static linking

## [0.4.2] - 2016-4-20

### Changed
- Bumped `libc` version to `0.2.10`

## [0.4.1] - 2016-4-2

### Changed
- Bumped `libc` version to `0.2.9`
- Bumped `rustc_version` version to `0.1.7`

## [0.4.0] - 2016-3-28

### Removed
- Removed support for `clang` 3.4.x

## [0.3.1] - 2016-3-21

### Added
- Added support for finding `libclang`

## [0.3.0] - 2016-3-16

### Removed
- Removed build system types and functions

### Added
- Added support for `clang` 3.4.x

### Changed
- Bumped `bitflags` version to `0.5.0`
- Bumped `libc` version to `0.2.8`

## [0.2.1] - 2016-2-13

### Changed
- Simplified internal usage of conditional compilation
- Bumped `bitflags` version to `0.4.0`
- Bumped `libc` version to `0.2.7`
- Bumped `rustc_version` version to `0.1.6`

## [0.2.0] - 2016-2-12

### Added
- Added support for `clang` 3.8.x

## [0.1.2] - 2015-12-29

### Added
- Added derivations of `Debug` for FFI structs

## [0.1.1] - 2015-12-26

### Added
- Added derivations of `PartialOrd` and `Ord` for FFI enums

## [0.1.0] - 2015-12-22
- Initial release
