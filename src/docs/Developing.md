# Developing the server #
[TOC]

## Dependencies ##

This project has 3 dependencies

- [json for modern C++](https://github.com/nlohmann/json) for JSON support (header only)
- [jsonrpcpp](https://github.com/badaix/jsonrpcpp) for JSON-RPC support (header only)
- [boost](https://www.boost.org)

-----------------

## How to build ##

This project was created on Windows 64bit and is not tested to compile/work on other systems but will probably work after some configuration, as all dependencies and the project's sources are cross-platform.

We use [CMake](https://cmake.org) to build the project together with the [mingw-w64](http://mingw-w64.org/doku.php/start) toolchain.

You can either build the server using the [VSCode CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) or manually.

### Manual install ###

~~~~~~~~~~~~~~~~~~~~~~~
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make.exe
~~~~~~~~~~~~~~~~~~~~~~~

### Install using CMake Tools ###

1. Open the repository as a workspace in VSCode.
2. Open CMakeLists.txt in the root directory.
3. Save, CMake Tools should now configure the build files.
Alternatively, configure the project manually.
4. Press F7 or click on build in the bottom row.

If errors occur, make sure to select the build target "ARXML_LanguageServer".

-----------------

## How to run / debug ##

The server itself does pretty much nothing on its own. It tries to connect to the socket 127.0.0.1 at a given port and closes if it can't connect.
The server is supposed to connect to an editor extension client implementing the [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) and responds to the extensions requests.

An editor extension client for Visual Studio code is available [here](https://github.com/JonasRock/ARXML_NavigationHelper), but it should work too with other Editors if the have Language Server Protocol support, but might require a change of transport for the protocol.

Aforementioned client extension will instantiate the server process on its own, making it really hard to debug, but can be configured to wait for an external process to connect instead.
This allows us to run the server ourselves with a debugger.
To configure the client to wait for an exteral process is currently only possible through commenting out

~~~~~~~~~~~~~~~~~~~~~~~~~
extension.ts::createServerWithSocket():

exec = child_process.spawn(executablePath)
~~~~~~~~~~~~~~~~~~~~~~~~~

in the clients extension.ts file

Now we can start a visual studio instance with the extension enabled, open a ARXML file and then start the server to connect to the extension.
In the Debug Log of the VSCode instance thats owning the Extension Development Host (The one you started the extension with) should now log what port number it will listen to. Either provide the number as an argument to the server or enter it manually after starting.

Note: Running or debugging the server using F5 does not work for me, but the buttons on the bottom row do.

-----------------

## Structure / Architecture ##

The following sections describe how the server functions internally and are thus only important when wanting to make changes to the server itself.

### Startup ###

The communication between server and client is using sockets. The client owns the connection and starts the server who connects to the client.

The client then requests initialization, then the server responds with his capabilites and provided features.
After an acknowledgement notification from the client the server is fully functional.

### Request Handling ###

The server waits until a message is sent to it, parses the message, calculates the response and sends it back. This loop is synchronous, the server can only process the next message after the response to the first one is sent back.

For every request/notification we can receive based on our initialization, we register a callback function in main() to the JSON-RPC parser that handles the requests.

The parser provides the arguments for the request to the callback, that processes the request, formulates a result and passes it back to the parser, that serializes it and passes it back to the main to be sent back out to the client via socket.

The data flow can be visualized as such:
~~~~~~~~~~~~~~~~
Request:

readocket -> JSON-RPC-Parser -> callback -> xmlParser -> shortnameStorage
~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~
Response:

writeSocket <- JSON-RPC-Parser <- callback <- xmlParser <- shortnameStorage
~~~~~~~~~~~~~~~~

### Parsing the ARXML file ###

When first asked for a definition or request location, the server memorymaps the file and parses it. After it has built its internal data structures, it closes the file again, and does not need to reopen it to process requests in that file, as future requests will only require the internal data.

This data is managed by shortnameStorage and
owned by the xmlParser.

The callbacks get their results from the parser.

-----------------

## How to add functionality ##

To add other features of the Language Server Protocol, multiple steps are required:

1. Register the functionality in the methods::request_initialise() function according to the [Language Server Protocol Specification](https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize)\n
For example, to add hovertext to the server add the hoverProvider like this:
~~~~~~~~~~~~~~~~~~~~~~~
methods.cpp:methods::request_initialize():

json result = {
    {"capabilities", {
        {"referencesProvider", true},
        {"definitionProvider", true},
+ + + + {"hoverProvider", true} + + + +
    }}
};
~~~~~~~~~~~~~~~~~~~~~~~

2. Create the corresponding callback():\n
Don't forget to add the prototpye to methods.hpp\n
The Response json object must be formatted according to LSP specification
~~~~~~~~~~~~~~~~~~~~~~~
methods.cpp:

jsonrpcpp::response_ptr methods::request_textDocument_hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    json respone = nullptr;
    // call xmlParser for info from here
    // do calculations etc...
    return std::make_shared<jsonrpcpp::Response>(id, result);
}
~~~~~~~~~~~~~~~~~~~~~~~

3. If needed, extend the xmlParser

4. register your callback in main():
~~~~~~~~~~~~~~~~~~~~~~~
main.cpp:main():

parser.register_request_callback("textDocument/hover", methods::request_textDocument_hover);
~~~~~~~~~~~~~~~~~~~~~~~

To make your life easier, you can declare the json interfaces from the LSP specification as structs int types.hpp and add the macro like for the others.
This enables you to just assign a json object with a struct and the other way around, so you can work with cpp data structures and convert it to json for the return value. For example:
~~~~~~~~~~~~~~~~~~~~~~~
types.hpp::lsp:

struct Position
{
    uint32_t line;
    uint32_t character;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position, line, character)
~~~~~~~~~~~~~~~~~~~~~~~
For further documentation on these structs, see [the JSON for modern C++ documentation](https://github.com/nlohmann/json#arbitrary-types-conversions)

## Further resources ##

- [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)
- [VSCode Language Server Guide](https://code.visualstudio.com/api/language-extensions/language-server-extension-guide)
- [jsonrpcpp](https://github.com/badaix/jsonrpcpp)
- [JSON for modern C++](https://github.com/nlohmann/jsonn)