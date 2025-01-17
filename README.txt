# File Transfer Server and Client

## Overview
This project implements a multi-process and multi-threaded file transfer server and client system using C and Linux. The primary goal is to facilitate concurrent file transfer between a server and multiple clients while practicing concepts such as process creation, multi-threaded programming, inter-process communication (IPC), POSIX message queues, and named pipes.

The project is divided into two parts:
1. **Part A:** Multi-process implementation
2. **Part B:** Multi-threaded implementation

## Features
1. **Concurrent File Transfers:**
   - Multiple clients can interact with the server simultaneously.
   - For each client, the server spawns a separate process (Part A) or thread (Part B).

2. **Command-Based Interaction:**
   - `list`: Lists all files in the server's directory.
   - `get <FILENAME> <LOCALFILENAME>`: Downloads a file from the server.
   - `quit`: Terminates the client and its associated process/thread on the server.

3. **POSIX Message Queues and Named Pipes:**
   - Message queues handle client-server connection requests.
   - Named pipes facilitate client-server communication.

4. **Termination Program:**
   - A `ftterminate` program to cleanly terminate the server and all associated processes, ensuring no residual message queues or pipes.

## Usage

### 1. Compilation
Use the provided `Makefile` to compile the programs:
```bash
make
