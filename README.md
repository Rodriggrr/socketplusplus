# C++ Socket Programming library

## Introduction

This library is a wrapper around the C socket API. It provides a simple namespace to create a server or a client.

## Usage

See example files. All the functions are documented in the header files.

## Compile

- **On Windows:**

    ```bash
    g++ <file.cpp> -o <file.exe> -lws2_32 # link with the winsock library
    ```

- **On Linux:**

    ```bash
    g++ <file.cpp> -o <file>
    ```

## License

SEE LICENSE FILE