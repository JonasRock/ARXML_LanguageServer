# Developing the server #
[TOC]

## Dependencies ##

This project has 3 dependencies

- [json for modern C++](https://github.com/nlohmann/json) for JSON support (header only)
- [jsonrpcpp](https://github.com/badaix/jsonrpcpp) for JSON-RPC support (header only)
- [boost](https://www.boost.org)

-----------------

## How to build ##

The Server can be built on Windows using MinGW (I can't get it to compile with MSVC) and on Linux using GCC.

[CMake](https://cmake.org) is used to build the project together with the [mingw-w64](http://mingw-w64.org/doku.php/start) toolchain.

You can either build the server using the [VSCode CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) or manually.

For Linux the build is automated with Azure Pipelines. (Windows Pipeline does not work because Boost is only availably for MSVC there and MSVC doesn't want to compile this)

### Manual install ###

~~~~~~~~~~~~~~~~~~~~~~~
mkdir build
cd build

cmake ..
cmake --build .
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

The server itself does pretty much nothing on its own. It tries to connect to 127.0.0.1 at a given port and closes if it can't connect.
The server is supposed to connect to an editor extension client implementing the [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) and responds to the extensions requests.

An editor extension client for Visual Studio code is available [here](https://github.com/JonasRock/ARXML_NavigationHelper), but it should work too with other Editors if the have Language Server Protocol support, but might require a change of transport for the protocol.

Aforementioned client extension will instantiate the server process on its own, making it really hard to debug, but can be configured to wait for an external process to connect instead.
This allows us to run the server ourselves manually with a debugger.
To configure the client to wait for an exteral process is currently only possible through commenting out

~~~~~~~~~~~~~~~~~~~~~~~~~ts
//extension.ts: createServerWithSocket():

exec = child_process.spawn(executablePath)
~~~~~~~~~~~~~~~~~~~~~~~~~

in the clients extension.ts file

Now we can start a VSCode instance with the extension enabled (F5), open a ARXML file (so the extension starts) and then start the server to connect to the extension.
In the Debug Log of the VSCode instance thats owning the Extension Development Host (The one you started the extension with) should now log what port number VSCode will listen to. Either provide the port as an argument to the server or enter it manually after starting.

-----------------

## Structure / Architecture ##

The following sections describe how the server functions internally and are thus only important when wanting to make changes to the server itself.

### Structure ###

- lsp::IOHandler: Manages the connection to the socket, as well as reading and writing
- lsp::LanguageService: Contains main routine and all callbacks
- lsp::MessageParser: Manages parsing of messages and management of corresponding callbacks
- lsp::XmlParser: Handles processing of arxml files and provides file information
- lsp::ArxmlStorage: Data structure used for holding parsed arxml data

### Startup ###

The communication between server and client is using sockets. The client owns the connection and starts the server who connects to the client.

The client then requests initialization, then the server responds with his capabilites and provided features.
After an acknowledgement notification from the client the server is fully functional.

### Request Handling ###

The server waits until a message is sent to it, parses the message, calculates the responses and sends them back. This loop is synchronous, the server can only process the next message after the responses to the first one are sent back. Multiple responses from the server to the client are supported

For every request/notification we can receive based on our initialization, we register a callback function in the lsp::LanguageService that owns the lsp::MessageParser that handles the requests.

The lsp::MessageParser provides the arguments for the request to the callback, that processes the request, formulates a result and passes it back to the parser, that serializes it and passes it back to the main to be sent back out to the client via socket.

The data flow can be visualized as such:

![](src/docs/img/LanguageServerRequestSequence.png)

### Request to the Client ###

The server can also request information from the client. To do this, you can add additional request to be sent back together with the original response in another callback. For example, with getting the Configuration after initialization, it works like this:

![](src/docs/img/ConfigurationResponseSequence.png)

A more in depth explantaion how to use this is available further down.

### Parsing the ARXML files ###

On the first request, the server memorymaps the file(s) and parses it (with the lsp::XmlParser). After it has built its internal data structure (lsp::ArxmlStorage), it closes the file again, and does not need to reopen it to process requests in that file, as future requests will only require the internal data.

This data is managed by lsp::ArxmlStorage and
owned by the lsp::XmlParser.

The callbacks get their results from the lsp::XmlParser.

-----------------

## How to add functionality ##

To add other features of the Language Server Protocol, multiple steps are required:

### Notifications and Requests to the server (from VSCode) ###

1. **Register the functionality**:\n
In the lsp::LanguageService::request_initialise() function according to the [Language Server Protocol Specification](https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize)\n
For example, to add hovertext to the server add the "hoverProvider" capability like this:
~~~~~~~~~~~~~~~~~~~~~~~cpp
//languageService.cpp: lsp::LanguageService::request_initialize():

json result = {
    {"capabilities", {
        {"referencesProvider", true},
        {"definitionProvider", true},
+ + + + {"hoverProvider", true} + + + +
    }}
};
~~~~~~~~~~~~~~~~~~~~~~~

2. **Create the corresponding callback**:\n
The callback functions are owned by the lsp::LangageService
The Response json object returned must be formatted according to LSP specification.\n
No return value is needed when dealing with a notification.
~~~~~~~~~~~~~~~~~~~~~~~cpp
//languageService.hpp: lsp::LanguageService:

static jsonrpcpp::response_ptr request_textDocument_hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);


//languageService.cpp:

jsonrpcpp::response_ptr lsp::LanguageService::request_textDocument_hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    json respone = nullptr;
    // call xmlParser for info from here
    // do calculations etc...
    return std::make_shared<jsonrpcpp::Response>(id, result);
}
~~~~~~~~~~~~~~~~~~~~~~~

3. If needed, **extend the lsp::XmlParser**:\n
The lsp::XmlParser is the class to interface with to get information about the parsed data.

4. **Register your callback**:\n
Response/Notification callbacks are registered in lsp::LanguageService::start(), which gets called once upon starting the server.
~~~~~~~~~~~~~~~~~~~~~~~cpp
//languageService.cpp: lsp::LanguageService::start():

parser.register_request_callback("textDocument/hover", request_textDocument_hover);
~~~~~~~~~~~~~~~~~~~~~~~

### Notifications and Requests to the client (to VSCode)

You can also send requests to the server and react to their response.
To do this, first, create a function that creates the request and writes the json dump of the request to the lsp::ioHandler. Then we need to register a callback to react to the response. Instead of calling the callbacks by method, when dealing with responses, the lsp::MessageHandler instead calls the correct method by the give request ID, and clears the callback association afterwards.

Notifications to the client work the same way, but you don't need to handle a response.

1. **Create the function** to send the request in lsp::LanguageService.\n
This function should construct and send the message to the lsp::IOHandler, as well register a callback for the response, if applicable
~~~~~~~~~~~~~~~~~~~~~~~cpp
//LanguageService.cpp:

void lsp::LanguageServer::toClient_request_workspaceFolders()
{
    //Parameters need to be formed according to LSP specification
    json parameters = nullptr;
    //The id is used to associate the response to the correct callback and 
    //should be unique, so we use getNextRequestID() to generate it
    jsonrpcpp::Id id(getNextRequestID());
    jsonrpcpp::Request request(id, "request/workspaceFolders", parameters);

    ioHandler_->addMessagesToSend(request.to_json().dump());
    messageParser_->register_response_callback(id.int_id(), response_workspaceFolders);
}
~~~~~~~~~~~~~~~~~~~~~~~

2. **Create the response callback**
~~~~~~~~~~~~~~~~~~~~~~~cpp
//LanguageService.cpp:

void lsp::LanguageServer::response_workspaceFolders(jsonrpcpp::Parameter &params)
{
    //Do something with the returned values;
}
~~~~~~~~~~~~~~~~~~~~~~~

You can use the created function (the first one, not the response callback) in other LanguageService callbacks, for example, when you want to request something after the initialization, you can call it, from the lsp::LanguageService::notification_initialized() callback

### LSP Types ###

To make your life easier, you can declare the json interfaces from the LSP specification as structs int types.hpp and add the macro like for the others.
This enables you to just assign a json object with a struct and the other way around, so you can work with cpp data structures and convert it to json for the return value. For example:
~~~~~~~~~~~~~~~~~~~~~~~cpp
//types.hpp: lsp::types:

struct Position
{
    uint32_t line;
    uint32_t character;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position, line, character)
~~~~~~~~~~~~~~~~~~~~~~~
For further documentation on these structs, see [the JSON for modern C++ documentation](https://github.com/nlohmann/json#arbitrary-types-conversions)

## Features not in the Language Server Protcol ##

- The **Tree View** feature of the VSCode extension this was developed for is not possible to implement using just the Language Server Protocol, so the communication for the treeView requests does not follow Language Server Protocol Specification (since it does not exist).

- Although **event logging** is specified in the Language Server Protocol, its not specified what the client does with the logged data, so this feature also requires a compatible client.

### Tree View ###

The Tree View is registered clientside and uses the existing connection to the server for requests. These requests can be handled like any other LSP request, its just not specified by the LSP specification.

When the server receives a treeView/getChildren request (The method name here is just what I chose, as there's no specification), it queries the xmlParser like the other requests, and sends the information about the children back. \n
This includes some metadata about the elements to allow the extension to make the jump to definition and hover features in the treeView possible clientside. \n
These are **NOT** the same as their LSP equivalents, and are implemented in the client side extension

### Event Logging ###

The telemetry/event notification is specified by the language server protocol, but there is no specification on how this is used clientside. The client extension for this server uses it to display log information to the user.

To send a log to the user, use the provided function
~~~~~~~~~~~~~~~~~~~~~~~~cpp
//languageService.hpp: lsp::LanguageService:

static void toClient_notification_telemetry_event(json params);
~~~~~~~~~~~~~~~~~~~~~~~~

## ARXML Parser ##

The lsp::XmlParser is responsible for getting the data in the .arxml file into a a lsp::ArxmlStorage and serving the stored data to requests from the lsp::LanguageService.

This parser is **NOT** an XML parser by specification. It is nowhere near specification compliant nor validating in any way. Essentially, this parser is specifically written for parsing exactly the data needed for the server to work but is not suited for anything else, as it has no idea of the actual structure of the file and just analyses the text.

A consequence of this is that the parser might or might not work on ill-formed documents and can only find the most basic errors, and probably not even those.

The parser works as follows:

1. **On receiving any request** for data, or a request to preparse a file or folder, the parser looks if there's already data for the given resource stored. If yes, it serves the request with the data already in memory and does not parse the files again.

2. If no data in memory was found, the parser creates a lsp::ArxmlStorage data structure to store the parsed data for the requested resources and begins parsing:

    1. The parser **memorymaps the file**
    2. The parser **scans the document for all newlines**, and saves their offsets. This is needed to convert from LSP Positions that give a line number and a character offset to a pure offset from the start of the file
    3. The parser scans the document again, **analysing each xml element** and keeping track of the current depth. On encountering either a SHORT-NAME or a reference, it stores the relevant info for that element in the lsp::ArxmlStorage, including position, name, children, parents, etc.
    4. The parser frees the memorymapped file. It will not reopen the file for other request unless the lsp::ArxmlStorage for this file is removed

    **Note**: Since the server is not multithreaded, parsing halts execution until the parsing is finished. Requests from the server will receive delayed responses, as the server only resumes answering after parsing.

## Further resources ##

- [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)
- [VSCode Language Server Guide](https://code.visualstudio.com/api/language-extensions/language-server-extension-guide)
- [jsonrpcpp](https://github.com/badaix/jsonrpcpp)
- [JSON for modern C++](https://github.com/nlohmann/jsonn)
- [boost](https://www.boost.org)
- [CMake](https://cmake.org)