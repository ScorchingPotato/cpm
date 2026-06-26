# cpm (C Project Manager)

A minimal, fast build automation tool and package manager designed specifically for C projects on Linux. 

## Features
* **Project Scaffolding:** Sets up structured workspace paths (`src`, `include`, `lib`) with one command.
* **Incremental Rebuilds:** Built-in compiler engine that compiles only modified source files.
* **Dependency Automation:** Fetches headers, sources, and libraries directly from remote URLs via `libcurl`.
* **Build Backends:** Supports its internal build runner or exports automatically to GNU Make.
