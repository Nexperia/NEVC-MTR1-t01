<picture align="center">
  <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/Nexperia/NEVC-MCTRL-100-t01/main/docs/img/nexperia_logo_white.svg">
  <img alt="Nexperia Logo" src="https://www.nexperia.com/.resources/nexperia-theme/images/logo.png">
</picture>

-----------------
# NEVC-MTR1-t01: Trapezoidal control of brushless DC (BLDC) motors using hall effect sensors firmware for NEVB-MTR1 series evaluation kit 

![Version](https://img.shields.io/badge/Version-1.0.0-blue) [![License - MIT/X Consortium](https://img.shields.io/badge/License-MIT%2FX%20Consortium-green)](https://github.com/Nexperia/NEVB-MTR1-t01/blob/main/LICENSE)

## Introduction

This project is focused on the implementation of motor control, specifically
designed for Brushless DC (BLDC) motors using Hall Effect sensors on the
LEONARDO board or the ATMEGA32u4 micro controller, platforms widely recognized
under the Arduino Leonardo nomenclature when operated through the Arduino IDE.

This code is compatible with Nexperia's Motor evaluation kit
NEVB-MTR1-KIT1, which includes the following components:

- Motor Controller Board (NEVB-MTR1-C-1): The central unit for motor control
  operations.
- 3-Phase Inverter Board (NEVB-MTR1-I56-1): Facilitates the conversion of DC to
  AC power for the motor.
- Leonardo R3 Development Board: Acts as the core micro controller platform for
  the system.
- 3-Phase BLDC Motor with Hall Sensors: The motor that is
  directly controlled by the system.
- Additional screws, plugs, and tools necessary for setup and operation.

To fully understand the system's capabilities and limitations, and for detailed
instructions on setting up the kit, please refer to the accompanying user
manual. This repository focuses on providing the code documentation and the
essential software to be uploaded to the Leonardo development board for use with
the above-mentioned hardware. 

To adapt and utilize this code for specific requirements, please review the
'todo' section of the documentation. This section outlines the expected user
modifications to tailor the software to individual needs. The default settings
in the code are optimized for the motor included in the kit, ensuring a smooth
start-up and operation for new users.

## Documentation 

The official documentation is hosted on [https://nexperia.github.io/NEVC-MTR1-t01/](https://nexperia.github.io/NEVC-MTR1-t01/).

## Generating Documentation (from source)

### Prerequisites

To generate the documentation, run the following command in the root directory of this repository:

Download and install [Doxygen](https://www.doxygen.nl/) (1.9.7 or newer) or use
package managers like apt or brew for Linux or macOS. 

```
# For Ubuntu/Debian
sudo apt-get install doxygen

# For macOS
brew install doxygen
```

If Doxygen is installed in a location that is not in your system's PATH, you
may need to add the Doxygen binary path to your PATH environment variable.

Optionally, download [Graphviz](https://graphviz.org/) or use package manager
commands to install. Doxygen can generate graphical class hierarchy diagrams if
Graphviz is installed. Alternatively, you can disable the use of dot tool for
generating graphs in the `Doxyfile` by setting the option `HAVE_DOT`.

```
HAVE_DOT = NO
```

The theme for the documentation is in the `docs/theme/` directory but is it 
not in the file tree, then need to be cloned from the git repository as a
submodule by using the commands:

```
git submodule init 
git submodule update
```

Once the Doxygen is installed, run the following command to generate the
documentation:

```
doxygen ./Doxyfile
```

This command will process the source code based on the configurations in the
Doxyfile and generate documentation files in the specified output directory
which should be `docs/html/`.

## Environments

To use this project, Arduino drivers need to be installed. They can be installed
separately or together with the installation of Arduino IDE v1 or v2. 

If using Visual Studio Code, please bear in mind the Arduino extension must also
be installed and the `.vscode/c_cpp_properties.json` configuration file must be
set up correctly for Intellisense to work properly. If Arduino v1 is installed,
run the PowerShell script `create_cpp_properties.ps1` (for Windows) or Bash
script `create_cpp_properties.sh` (for Linux) and this takes care of setting up
the configuration file.

## Usage

- Upload main.ino:
  - Open file in the editor of choice and upload main.ino to a compatible micro
    controller.
  - This code is optimized for the Arduino Leonardo (LEONARDO board or
    ATMEGA32u4 micro controller), but it might be customized to be compatible
    with other Arduino boards.

- Refer to the User Manual for setting up the kit:
  - Before powering on the system, it's essential to refer to the user manual
    for specific setup instructions related to the Nexperia's Motor evaluation
    kit.
  - The manual provides critical information on how to properly configure and
    prepare the system for operation.

- Power On the System:
  - After ensuring that the setup is in accordance with the manual, power on the
    system.
  - The motor control program will start, enabling the control and operation of
    the BLDC motor using the Hall Effect sensors.

## Safety Precautions

- Motor Handling: Ensure that the motor is securely clamped down and cannot move
  unexpectedly during operation. This is crucial to prevent any accidents or
  damage to the system.

- Operating Ranges: Verify that the motor's intended operating ranges (voltage,
  current, speed, etc.) are supported by your setup. Operating the motor outside
  its specified limits can lead to malfunction or damage.

- Power Isolation: Implement appropriate measures to quickly and safely isolate
  the power to the system if needed. This might include emergency stop
  mechanisms or easily accessible power switches.

- General Safety: Always take reasonable safety precautions when handling
  electronic components. This includes wearing protective gear as necessary,
  ensuring a tidy and organized workspace, and being cautious of potential
  electrical hazards.

## Contributing

Contributions to this project are welcome. To contribute:

- Fork the repository.
- Create a new branch for your feature or fix.
- Commit your changes.
- Push to your branch and submit a pull request.

## License

This project is licensed under the MIT/X Consortium License, a permissive free
software license. For more details on the MIT/X Consortium License, please refer
to the LICENSE file included in this repository.
