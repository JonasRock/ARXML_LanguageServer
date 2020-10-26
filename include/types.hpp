#ifndef TYPES_H
#define TYPES_H

#include "json.hpp"

//Contains the Structures defined by the Language Server Protocol as C++ Structs for easier handling
//These can be converted to json by simply assigning "json = struct;" and the other way around
namespace lsp
{
    typedef std::string DocumentUri;

    struct Position
    {
        uint32_t line;
        uint32_t character;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position, line, character)

    struct Range
    {
        lsp::Position start;
        lsp::Position end;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Range, start, end)

    struct Location
    {
        lsp::DocumentUri uri;
        lsp::Range range;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Location, uri, range)

    struct LocationLink
    {
        lsp::Range originSelectionRange;
        lsp::DocumentUri targetUri;
        lsp::Range targetRange;
        lsp::Range targetSelectionRange;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LocationLink, originSelectionRange, targetUri, targetRange, targetSelectionRange)

    struct TextDocumentIdentifier
    {
        lsp::DocumentUri uri;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextDocumentIdentifier, uri)

    struct TextDocumentPositionParams
    {
        lsp::TextDocumentIdentifier textDocument;
        lsp::Position position;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextDocumentPositionParams, textDocument, position)

}
#endif /* TYPES_H */