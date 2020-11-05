# ARXML Navigation Helper #

[TOC]

This extension provides easier navigation of Autosar XML files.

## Prerequesites ##

The language server used for resolving commands is Windows-x64 only at the moment.

For bigger files (>50MB) VSCode disables extensions. To get around this, the extension [Large file support for extensions](https://marketplace.visualstudio.com/items?itemName=mbehr1.vsc-lfs) is needed.

---------------

## Features ##

- **Go to definition**: Jump to the corresponding SHORT-NAME element by clicking on the corresponding path part on a REF element
- **Go to references**: show all REF elements that point to this SHORT-NAME element

---------------

## Useful Commands and Shortcuts ##

- **Ctrl + LeftClick** as a shortcut for **go to definition**
- **Alt + LeftArrow** return to your last position ("undo jump")
- Inner parts of REF elements can be jumped to individually (go to references works there too)

---------------

## Common issues ##

### Extension does not work on big files ###

For files bigger than around 50mb, Visual Studio Code disables all extensions automatically.
The current workaround for this is to use the VSCode extension
[Large file support for extensions](https://marketplace.visualstudio.com/items?itemName=mbehr1.vsc-lfs).
Install it and use to command (Ctrl+Shift+P) **open large file...**
This can currently only open the file in readonly mode.

### Jumping takes very long ###

When first executing a jump on a file, the file has to be parsed by the extension.
This takes a couple of seconds for bigger files (around 5 seconds for 170MB for me).
Afterwards jumps to definition should be almost instant.
Things can take a while to load, especially on bigger files because VSCode needs to load in all the surrounding areas for the references, the language server itself cannot speed that up.

### No definition found ###

Some REFs may link to elements that are not in the current file, as Autosar allows these files to be split up into multiple files.
Currently only file-local references are supported.

## Developing the server ##

[Read Here](src/docs/Developing.md)