# WallShell

[![GitHub License](https://img.shields.io/github/license/MalTheLegend104/WallShell)](https://www.apache.org/licenses/LICENSE-2.0)
![Build Workflow Status](https://img.shields.io/github/actions/workflow/status/MalTheLegend104/WallShell/build-test.yml)
[![Current Release](https://img.shields.io/github/v/release/MalTheLegend104/WallShell)](https://https://github.com/MalTheLegend104/WallShell/releases/latest)
![Commits since latest release](https://img.shields.io/github/commits-since/MalTheLegend104/WallShell/latest)

An easy-to-use, highly portable, CLI using C99.

## Table of Contents

- [Portability and Use Cases](#portability-and-use-cases)
  - [Freestanding Environments](#freestanding-environments)
  - [Large Scale Projects](#large-scale-projects)
  - [General Uses](#in-general)
- [Usage](#usage)
- [The Name](#the-name)
- [License](#license)

## Portability and Use Cases

### Freestanding Environments

A big focus of WallShell was to create an easily portable shell for freestanding environments.
With proper configuration, WallShell only requires a very minimal portion of the standard library.
This is great for embedded systems, hobby operating systems, and any other low-level application.

### Large Scale Projects

Customizations make WallShell applicable to large-scale applications as well.
Some example use cases:

- Simple Debug Console for Development
  - Can easily run alongside a full-scale GUI and be used to configure application state, configurations, monitoring
    internal tasks, etc.
- Frontend for Backend Applications
  - Can act as a frontend interface for managing and monitoring the application's internal state, configurations, and
    performing administrative tasks, all without the burden of creating a full-scale GUI.
  - It allows for quick prototyping, testing, etc., allowing you to see if an idea works before committing to writing a full GUI.

### In General

WallShell provides many great cross-platform wrappers around things like cursor control and console colors.
You can use however much (or little) of WallShell as you want. It's highly adaptable to any potential use cases.

The sky's the limit. You define the commands that are run and what they do. WallShell only needs a function pointer.

## Usage

This list below lists everywhere you can find information about using WallShell in your own projects.

- [Getting Started](docs/getting_started.md)
- [General Purpose Docs](docs/README.md)
- [In-Depth Examples](examples/README.md)
- [FAQ](docs/FAQ.md)
- [Contribution Guide](docs/contributing.md)
- All functions (save for a few internal ones) are documented using doxygen comments.
  - The doxygen documentation can be generated yourself or found [here](http://malthelegend104.github.io/WallShell).

## The Name

You may be wondering what's up with the name. Where did "Wall" come from? And is it really a shell?

- The "Wall"
  - When coming up with a name for the project, I looked up from my computer screen and saw a wall. It was supposed to be temporary, but it grew on me.
- Is it really a "shell"?
  - That depends on how you implement it. It can act as a normal CLI, or be built into an OS to make a fully-featured shell.
  - It sits between a terminal and an application, and, in my mind, that's close enough to what a shell does to be called that.

## License

Copyright 2025 [MalTheLegend104](https://github.com/MalTheLegend104/)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
