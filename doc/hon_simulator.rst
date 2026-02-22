hon_simulator
==============

Overview
--------

`hon_simulator` is a small simulator for devices that use the hOn-style
Haier protocol. It is intended to help developers test host software and
transport implementations without requiring physical appliances.

Location
--------

Sources live in the tools tree: `tools/hon_simulator`_

Build
-----

Prerequisites: CMake and a C++ compiler (Visual Studio on Windows, GCC/Clang on Linux/macOS).

Windows (recommended)

.. code-block:: powershell

  mkdir build
  cd build
  cmake -G "Visual Studio 17 2022" ..
  cmake --build . --config Release

Linux / macOS

.. code-block:: sh

  mkdir build && cd build
  cmake ..
  make -j$(nproc)

The resulting executable will be in the build output directory (e.g. ``build/Release`` or ``build/``).

Usage
-----

Run the built executable from the build directory. The simulator accepts runtime
options (serial device/port, logging verbosity, etc.). Use the built-in help to
discover supported flags:

.. code-block:: sh

  ./hon_simulator --help

Example (Windows)

.. code-block:: powershell

  .\Release\hon_simulator.exe

Notes
-----

The simulator is meant for manual testing and continuous integration scenarios.
Consult the executable's ``--help`` output for exact runtime options and flags.
