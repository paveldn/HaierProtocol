smartair2_simulator
===================

Overview
--------

`smartair2_simulator` provides a simulated SmartAir2-style Haier appliance
for testing hosts and transport layers that communicate with older units.

Location
--------

Sources live under: `tools/smartair2_simulator`_

Build
-----

Prerequisites: CMake and a C++ compiler.

Windows

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

Usage
-----

Run the compiled binary from the build directory. The simulator accepts common
runtime options (serial port/device, verbosity, configuration). Run with
``--help`` to list supported options:

.. code-block:: sh

  ./smartair2_simulator --help

Example (Linux)

.. code-block:: sh

  ./smartair2_simulator /dev/ttyUSB0 115200

Notes
-----

The simulator is intended for development and automated tests. For exact
command-line parameters check the executable's help output.
