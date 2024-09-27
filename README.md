# Multithreaded Command Processing System

## Overview

This project simulates a multithreaded server that processes commands from clients. Each client issues commands to add, remove, or print integers for specific users. Commands are processed in the order they are received, while ensuring thread-safety through mutexes and efficient command handling via priority queues and data structures like `multiset`, `unordered_map`, and `forward_list`.

The server handles multiple clients issuing commands concurrently, and the execution of commands is based on their timestamp. Additionally, the server replays historical commands when necessary, ensuring consistency in user operations.

## Key Features
- **Multithreaded Server**: Handles multiple clients concurrently using threads.
- **Command Queue**: Commands are queued in a priority queue based on their timestamp.
- **Thread Safety**: Mutexes are used to ensure thread-safe access to shared data.
- **Command Types**:
  - **Add**: Adds an integer to the user's dataset.
  - **Remove**: Removes integers from the dataset.
  - **Print**: Prints the user's dataset, with optional range filtering.
- **Data Structures**:
  - **multiset**: Stores user data to maintain sorted integers with potential duplicates.
  - **forward_list**: Maintains a list of past commands to handle replay and prevent conflicts.
- **Client Simulation**: Clients simulate concurrent command entry via threads.

## Dependencies

This project uses standard C++ libraries for multithreading, synchronization, and data handling:
- `<iostream>`: For input/output operations.
- `<set>`: For `multiset` to store user data.
- `<unordered_map>`: For mapping user names to their data and mutexes.
- `<map>`: Used internally for timestamp comparison.
- `<mutex>`: For thread synchronization.
- `<vector>`: For parameter storage in commands.
- `<string>`: To handle user and command input.
- `<thread>`: For creating and managing multiple threads.
- `<chrono>`: For managing time points and timestamps.
- `<queue>`: For maintaining command queues.
- `<utility>`: For `pair` usage.
- `<forward_list>`: For storing past commands efficiently.

## How the System Works

### Classes

- **Command**:
  - Stores details of a single command (type, user, params, timestamp).
  - Contains functions to `add`, `remove`, and `print` data for a user.
  - Executes a command on user data.

- **Serwer**:
  - Acts as the main server that processes commands.
  - Stores a command queue (`priority_queue`), user data (`unordered_map`), and command history (`forward_list`).
  - Manages threads to process commands concurrently and maintains thread safety using multiple `mutex` objects.
  - Responsible for parsing client tasks, queueing commands, and managing user data.

- **Clients**:
  - Simulates multiple clients interacting with the server concurrently.
  - Each client takes tasks from a shared queue and sends them to the server for processing.

### Flow of Execution

1. **Server Initialization**:
   - The server is initialized with a set number of threads to process commands.
   - Each thread picks commands from the priority queue and executes them in order.
   
2. **Client Simulation**:
   - Clients are created and simulate tasks by sending commands (like `add`, `remove`, and `print`) to the server.
   - Tasks are picked from a shared queue and passed to the server for execution.

3. **Command Processing**:
   - Commands are queued based on their timestamp.
   - Threads process commands from the priority queue while ensuring that user data access is synchronized using mutexes.
   - Commands are executed (add, remove, print) based on the specified operation type and parameters.

4. **Concurrency and Thread Safety**:
   - Each user's data is protected by a unique mutex to prevent concurrent access issues.
   - The server manages multiple threads to concurrently process commands while maintaining the integrity of user operations.

### Command Examples

The following are some example tasks that can be processed by the system:

- `/user/add?10`: Adds `10` to the dataset of user `user`.
- `/user/remove`: Removes all values from the dataset of user `user`.
- `/user/remove?10`: Removes `10` from the dataset of user `user`.
- `/user/print`: Prints all values in the dataset of user `user`.
- `/user/print?0?11`: Prints all values in the range [0, 11] from the dataset of user `user`.
