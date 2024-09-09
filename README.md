# README: Synchronization and File Operations in C

This project focuses on advanced synchronization techniques and file operations in C, divided into four distinct parts. Each part builds upon the previous one to enhance understanding of concurrent programming, synchronization mechanisms, buffered I/O, and file system management.

## Part 1: Naive Synchronization of File Access

### Objective:
To handle file access synchronization using naive methods like `wait` and `sleep`. The challenge was to prevent interleaved file writes between a parent and two child processes.

### Implementation:
- Forked two child processes, which, along with the parent, wrote to the same file.
- Used naive synchronization methods (e.g., `sleep`, `wait`) to ensure that processes wrote in sequence, avoiding write collisions.
- Command-line arguments were used to define the messages and the number of writes each process performed.
- The program demonstrated how to ensure basic file writing order without using advanced locking mechanisms.

### Key Learning:
This task illustrated the limitations of basic synchronization methods like `sleep` and `wait`, particularly in more complex and unpredictable environments.

---

## Part 2: Synchronization with Lock Files

### Objective:
To enhance the synchronization approach by introducing a lock file mechanism, ensuring that only one process writes to the file at a time.

### Implementation:
- A dynamic number of child processes were forked, all writing to the same file.
- Implemented a lock file (`lockfile.lock`) to enforce mutual exclusion, ensuring that only one process accessed the file at a time.
- Processes used the lock file to coordinate access, creating the lock file before writing and removing it afterward.
- Command-line arguments were used to specify the messages and number of writes for each process.

### Key Learning:
This part demonstrated how a lock file can act as a simple but effective synchronization mechanism, preventing write collisions without complex locking primitives.

---

## Part 3: Buffered File I/O with Custom Flag

### Objective:
To implement a buffered I/O system that supports efficient file read/write operations and introduces a custom `O_PREAPPEND` flag to allow writing to the beginning of a file without overwriting existing content.

### Implementation:
- Developed a custom I/O library with buffered `read`, `write`, and `open` functions.
- The `O_PREAPPEND` flag was handled by reading the entire file into a buffer, inserting new content at the beginning, and appending the original content afterward.
- Buffered I/O reduced the number of system calls by batching writes, improving performance.

### Key Learning:
This part highlighted the importance of buffered I/O for performance improvements and showed how to implement custom file handling logic through user-defined flags like `O_PREAPPEND`.

---

## Part 4: Directory Copy Library in C

### Objective:
To implement a C library that provides functionality similar to Python's `shutil.copytree`, allowing recursive copying of directories, files, and symbolic links, with optional permission copying.

### Implementation:
- Implemented functions to recursively copy directories and handle symbolic links and file permissions.
- Used `copy_file` and `copy_directory` functions to manage copying of individual files and directories, with flags for handling symbolic links and file permissions.
- The main program allowed users to specify source and destination directories, along with optional flags to control the copying behavior.

### Key Learning:
This part demonstrated how to work with file system operations in C, focusing on directory traversal, file copying, and handling special file types like symbolic links.

---

## Conclusion

This project covered a broad range of file I/O and synchronization techniques, starting from naive methods, moving to lock-based synchronization, and finally implementing buffered I/O and complex file system operations. Each part provided key insights into concurrent programming challenges and solutions, improving both performance and correctness in file access scenarios.
