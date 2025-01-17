# CS342 Operating Systems - Project 1: File Transfer Server

## Overview

This project implements a multi-process and multi-threaded file transfer server and client system in C/Linux. The project explores key concepts like process creation, inter-process communication (IPC), multi-threading, and POSIX constructs, specifically message queues and named pipes.

---

## Project Structure

The project includes the following components:

1. **ftserver.c**: Multi-process server that handles client requests via child processes.
2. **ftclient.c**: Client program for interacting with the server.
3. **ftterminate.c**: Terminates the server and all related processes, cleaning up resources.
4. **tftserver.c**: Multi-threaded server version.
5. **tftclient.c**: Multi-threaded client program (same as ftclient.c but compiled separately).
6. **Makefile**: Automates compilation of all components.
7. **report.pdf**: Performance analysis of the implemented programs.

---

## Features

- Multi-process file transfer server using POSIX message queues and named pipes.
- Multi-threaded file transfer server.
- Supports concurrent handling of up to 10 clients.
- Commands:
  - `list`: Lists files in the server directory.
  - `get FILENAME LOCALFILENAME`: Downloads a file from the server.
  - `quit`: Terminates the client and its corresponding server process or thread.
- Clean termination using `ftterminate`.

---

## Usage

### Compilation
Run the following command to compile all components:
```bash
make
