
# ChronoTrigger - Task Scheduler

Chrono Trigger is a classic jRPG game where players travel through time to prevent a global catastrophe.
I played a lot of it as a kid on the SNES, but still, it's a great name for a task scheduler :)

## Building

This project includes both a static library and example programs that link to it. To build the project, follow the instructions below.

### Requirements

- A C++17 compatible compiler (e.g., `g++`).
- `make` for managing the build process.

### Clone the repository

First, clone the repository to your local machine:

```bash
git clone https://github.com/lukgni/chronotrigger.git
cd chronotrigger
```

### Build the project

The project includes two primary targets: **the library** (`lib`) and **the examples** (`examples`). You can build both using the default `all` target, or build them separately.

#### Build all components (library + examples)

To build the entire project, including the static library and the example programs, run:

```bash
make
```

This will:
1. Build the static library `libchronotrigger.a` located in the `build/lib` directory.
2. Compile all the example programs in the `examples` directory and place the executables in `build/bin/`.

#### Build only the library

To build only the static library, run:

```bash
make lib
```

This will:
1. Compile all source files from the `src` directory.
2. Create the static library `libchronotrigger.a` in the `build/lib/` directory.
3. Copy the header files from `include/` into the library's include directory (`build/lib/include/chronotrigger/`).

#### Build only the examples

To build only the example programs, run:

```bash
make examples
```

This will:

1. Compile the example source files in the `examples` directory.
2. Link them against the static library `libchronotrigger.a`.
3. Place the resulting executables in the `build/bin/` directory.

### Clean the project

To clean the build directory (remove all object files and binaries), run:

```bash
make clean
```

This will delete everything inside the `build/` directory.

### File Structure

After building the project, the file structure will look like this:

```
chronotrigger/
├── build/                # Directory for compiled files
│   ├── bin/              # Executables for examples
│   └── lib/              # Static library and include files
├── include/              # Header files
│   └── chronotrigger/    # Library-specific headers
├── src/                  # Source files for the library
├── examples/             # Example programs
├── Makefile              # Makefile for building the project
└── README.md             # Project documentation
```

---

If you encounter any issues or need further customization, refer to the `Makefile` or the project documentation for more details.
