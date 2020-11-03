# Using the extension #
[TOC]

## Prerequesites ##

The language server itself is currently Windows-x64 only. 

For bigger files (bigger than ~50mb), the VSCode extension [Large file support for extensions](https://marketplace.visualstudio.com/items?itemName=mbehr1.vsc-lfs) is needed.

-----------------------

## How to use ##

The Extension loads automatically on opening any ARXML file.\n
The first command takes some time as the server parses the file.

### Features ###

- **list/go to references** for shortname elements
- **go to definition** for references
- **go to definition** for specific path element

### Other useful Commands ###

- **Ctrl + LeftClick** is a shortcut for "go to definition"
- **Alt + LeftArrow** returns you to your last position ("undo jump")

-----------------------

## Common issues ##

### Extension does not load on big files ###

For files bigger than around 50mb, Visual Studio Code disables all extensions automatically.

The current workaround for this is to use the VSCode extension
[Large file support for extensions](https://marketplace.visualstudio.com/items?itemName=mbehr1.vsc-lfs).\n
Install it and use to command (Ctrl+Shift+P) **open large file...**

This can currently only open the file in readonly mode.

### No definition found error ###

ARXML files may be split up into multiple files. Currently only file-local references are supported.

The reference path might lie inside of an SD GID element. These are not parsed currently.

------------------------

## Developing the Server ##

[Read Here](src/docs/Developing.md)