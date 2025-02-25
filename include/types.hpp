/**
 * @file types.hpp
 * @author Jonas Rock
 * @brief Contains the Structures defined by the Language Server Protocol as C++ Structs for easier handling
 * @version 0.1
 * @date 2020-11-05
 */

#ifndef TYPES_H
#define TYPES_H

#include "json.hpp"

namespace lsp
{
namespace types
{
    enum arxmlError
    {
        multipleDefinitions
    };
    NLOHMANN_JSON_SERIALIZE_ENUM( arxmlError, {
        {lsp::types::arxmlError::multipleDefinitions, "multipleDefinitions"}
    })

    typedef std::string DocumentUri;

    struct Position
    {
        uint32_t line;
        uint32_t character;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position, line, character)

    struct Range
    {
        lsp::types::Position start;
        lsp::types::Position end;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Range, start, end)

    struct Location
    {
        lsp::types::DocumentUri uri;
        lsp::types::Range range;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Location, uri, range)

    struct Hover
    {
        std::string contents;
        lsp::types::Range range;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Hover, contents, range)

    struct LocationLink
    {
        lsp::types::Range originSelectionRange;
        lsp::types::DocumentUri targetUri;
        lsp::types::Range targetRange;
        lsp::types::Range targetSelectionRange;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LocationLink, originSelectionRange, targetUri, targetRange, targetSelectionRange)

    struct TextDocumentIdentifier
    {
        lsp::types::DocumentUri uri;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextDocumentIdentifier, uri)

    struct TextDocumentPositionParams
    {
        lsp::types::TextDocumentIdentifier textDocument;
        lsp::types::Position position;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextDocumentPositionParams, textDocument, position)

    struct ReferenceContext
    {
        bool includeDeclaration;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReferenceContext, includeDeclaration)

    struct ReferenceParams
    {
        lsp::types::TextDocumentIdentifier textDocument;
        lsp::types::Position position;
        lsp::types::ReferenceContext context;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReferenceParams, textDocument, position, context)

    struct DocumentColorParams
    {
        lsp::types::TextDocumentIdentifier textDocument;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentColorParams, textDocument)

    struct ConfigurationItem
    {
        std::string section;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigurationItem, section)

    struct ConfigurationParams
    {
        std::vector<ConfigurationItem> items;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigurationParams, items)

    namespace non_standard
    {
        struct ShortnameTreeElement
        {
            std::string name;
            std::string path;
            lsp::types::Position pos;
            std::string uri;
            uint8_t cState;
            bool unique;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShortnameTreeElement, name, path, pos, uri, cState, unique)

        struct GetChildrenParams
        {
            std::string path;
            std::string uri;
            bool unique;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GetChildrenParams, path, uri, unique)

        struct OwnerParams
        {
            lsp::types::Position pos;
            std::string uri;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OwnerParams, pos, uri)
    }

}
}
#endif /* TYPES_H */