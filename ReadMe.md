# File Transfer Server and Client

## Overview

This project implements a file transfer system in C/Linux. It consists of:
1. A **server** program that serves files from a directory.
2. A **client** program that allows users to interact with the server to list files, download files, and disconnect cleanly.

Two versions of the server are included:
- **Multi-Process Server:** Each client connection spawns a new process.
- **Multi-Threaded Server:** Each client connection spawns a new thread.

A termination program is also included to clean up all resources and processes when needed.

---

## Features

- **Concurrent Client Handling:** Supports up to 10 simultaneous clients.
- **Commands:**
  - `list`: List all files available in the server’s directory.
  - `get FILENAME LOCALFILENAME`: Download a file from the server.
  - `quit`: Disconnect the client and clean up resources.
- **File Handling:** Handles both text and binary files, treating all files as binary during transfer.
- **Termination:** Cleanly terminates all processes, threads, and resources using a dedicated termination program.

---

## Requirements

- **Operating System:** Linux (tested on Ubuntu 24.04 64-bit).
- **Tools:** gcc (for compiling the source code).
- **POSIX Constructs:** Message queues, named pipes, threads, and signals.

---

## Setup and Compilation

### Steps to Compile
1. Open a terminal and navigate to the project directory.
2. Run the following command:
   ```bash
   make
   ```
   This will compile the following programs:
   - `ftserver` (multi-process server)
   - `ftclient` (multi-process client)
   - `tftserver` (multi-threaded server)
   - `tftclient` (multi-threaded client)
   - `ftterminate` (termination program)

### Files Generated
- `ftserver`: Multi-process server executable.
- `ftclient`: Multi-process client executable.
- `tftserver`: Multi-threaded server executable.
- `tftclient`: Multi-threaded client executable.
- `ftterminate`: Program to terminate the server and clean up resources.

---

## Usage

### Multi-Process Server and Client

#### Starting the Server
Run the following command to start the server:
```bash
./ftserver DIRECTORY MQNAME SENDSIZE
```
- `DIRECTORY`: Path to the directory containing the files to be served.
- `MQNAME`: Name of the POSIX message queue used for client connections.
- `SENDSIZE`: Maximum size (in bytes) of each data block sent to the client.

Example:
```bash
./ftserver /home/user/files /ftconnqueue1 512
```

#### Starting a Client
Run the following command to start the client:
```bash
./ftclient MQNAME
```
- `MQNAME`: Name of the message queue used by the server.

Example:
```bash
./ftclient /ftconnqueue1
```

#### Client Commands
Once the client is running, you can use the following commands:
- `list`: Lists all files in the server’s directory.
- `get FILENAME LOCALFILENAME`: Downloads a file from the server and saves it as `LOCALFILENAME` in the client’s current directory.
  Example: `get example.txt myexample.txt`
- `quit`: Disconnects the client and terminates the associated server process.

---

### Multi-Threaded Server and Client

#### Starting the Server
Run the following command to start the threaded server:
```bash
./tftserver DIRECTORY MQNAME SENDSIZE
```
Example:
```bash
./tftserver /home/user/files /mq1 1024
```

#### Starting a Client
Run the following command to start the client:
```bash
./tftclient MQNAME
```
Example:
```bash
./tftclient /mq1
```

#### Client Commands
The commands for the threaded client are the same as the multi-process client.

---

### Termination Program
To cleanly terminate the server and all related processes or threads:
```bash
./ftterminate PID
```
- `PID`: Process ID of the main server process.

You can find the server PID using the `ps` command or by checking the server’s output when it starts.

Example:
```bash
./ftterminate 12345
```

---

## Experimentation

To evaluate the performance of the system, run experiments with:
- Different numbers of clients.
- Different file sizes.
- Different `SENDSIZE` values.

Measure the elapsed time for various operations, record the results, and analyze the trends. Detailed experimental results and interpretations are included in the accompanying `report.pdf` file.

---

## File Structure

- **ftserver.c:** Multi-process server source code.
- **ftclient.c:** Multi-process client source code.
- **tftserver.c:** Multi-threaded server source code.
- **tftclient.c:** Multi-threaded client source code.
- **ftterminate.c:** Termination program source code.
- **Makefile:** Compilation instructions.
- **report.pdf:** Experimental results and performance analysis.

---

## Notes

- This project was developed and tested on a single machine.
- Ensure proper cleanup of message queues and named pipes to avoid resource leakage.
- For any issues or questions, refer to the references below or reach out to the project maintainer.

---

## References

- [POSIX Message Queues](https://man7.org/linux/man-pages/man7/mq_overview.7.html)
- [Named Pipes](https://en.wikipedia.org/wiki/Named_pipe)
- [mkfifo](https://man7.org/linux/man-pages/man3/mkfifo.3.html)
- [Linux Signals](https://man7.org/linux/man-pages/man7/signal.7.html)

---

## Author
- Name: Emre Can Yologlu
- Course: CS342 - Operating Systems, Bilkent University

