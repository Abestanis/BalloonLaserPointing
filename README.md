# Balloon Laser Pointing

This project aims to develop a controller for a two motor platform that will allow to point
a laser at another balloon. The laser could then be used for communication, which is not
part of this project.


## Building

The project is based on [PlatformIO](https://platformio.org). To build the project, the
[PlatformIO command line tools](https://docs.platformio.org/en/latest/core/installation.html)
are required to be installed.

Run the following commands in the top level directory of the project
(the one containing this README and the [platformio.ini](platformio.ini) file).

Initialize the project and install the dependencies:
```shell
pio project init
pio lib install
```

Build the project:
```shell
pio run
```

Build and upload the project to a connected Arduino Due:
```shell
pio run --target upload
```


## Repository structure

* [`controller`](controller): Contains the controller program that can be used to control
                              the pointing system from a computer via a serial connection.
* [`images`](images): Images used for documentation.
* [`include`](include): C/C++ header files.
* [`lib`](lib): Project specific private libraries.
* [`models`](models): The 3D models of the laser pointing structure.
* [`src`](src): The C/C++ source files containing the code of the project.
* [`platformio.ini`](platformio.ini): [PlatformIO configuration file][platformio_config].


[platformio_config]: https://docs.platformio.org/en/stable/projectconf/index.html#projectconf
