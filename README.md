# WallShell

An easy-to-use, highly portable, CLI using C99.

## Portability and Use Cases

### Freestanding Environments 

A big focus of WallShell was to create an easily portable shell for freestanding environments.
With proper configuration, WallShell only requires a very minimal portion of the standard library.
This is great for embedded systems, hobby operating systems, and any other low level application.

### Large Scale Projects

Customizations make WallShell applicable to large scale applications as well.
Some example use cases:

- Simple Debug Console for Development
    - Can easily run alongside a full scale GUI and be used to configure application state, configurations, monitoring
      internal tasks, etc.
- Frontend for Backend Applications
    - Can act as a frontend interface for managing and monitoring the application's internal state, configurations, and
      performing administrative tasks, all without the burden of creating a full scale GUI.
    - Can act as a stopgap before you fully implement a GUI.
      - It allows for quick prototyping, testing, etc., allowing you to see if an idea works before committing to writing a full GUI.

There are plenty of ways to use WallShell. WallShell provides many great cross-platform wrappers around things like cursor control and console colors.
You can use however much (or little) of WallShell as you want. It's highly adaptable to any potential use cases.

The sky's the limit. You define the commands that are run and what they do. WallShell only needs a function pointer.

## Usage

This list below lists everywhere you can find information about using WallShell in your own projects.

- All functions (save for a few internal ones) are documented using doxygen comments.
- [Getting Started](docs/getting_started.md)
- [General Purpose Docs](docs/README.md)
- [In-Depth Example](main.c)
- [FAQ](docs/FAQ.md)
- [Contribution Guide](docs/contributing.md)

## The Name

You may be wondering what's up with the name. Where did "Wall" come from? And is it really a shell?

- The "Wall"
  - When coming up with a name for the project, I looked up from my computer screen and saw a wall. It was supposed to be temporary, but it grew on me.
- Is it really a "shell"?
  - That depends on how you implement it. It can act as a normal CLI, or be built into an OS to make a fully featured shell.
  - It sits between a terminal and an application, and, in my mind, that's close enough to what a shell does to be called that.
 
## License

(insert typical license stuff here)
