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
    /**
     * @brief LSP data type
     * 
     */
    typedef std::string DocumentUri;

    /**
     * @brief LSP data type
     * 
     */
    struct Position
    {
        uint32_t line;
        uint32_t character;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position, line, character)

    /**
     * @brief LSP data type
     * 
     */
    struct Range
    {
        lsp::Position start;
        lsp::Position end;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Range, start, end)

    /**
     * @brief LSP data type
     * 
     */
    struct Location
    {
        lsp::DocumentUri uri;
        lsp::Range range;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Location, uri, range)

    /**
     * @brief LSP data type
     * 
     */
    struct LocationLink
    {
        lsp::Range originSelectionRange;
        lsp::DocumentUri targetUri;
        lsp::Range targetRange;
        lsp::Range targetSelectionRange;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LocationLink, originSelectionRange, targetUri, targetRange, targetSelectionRange)

    /**
     * @brief LSP data type
     * 
     */
    struct TextDocumentIdentifier
    {
        lsp::DocumentUri uri;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextDocumentIdentifier, uri)

    /**
     * @brief LSP data type
     * 
     */
    struct TextDocumentPositionParams
    {
        lsp::TextDocumentIdentifier textDocument;
        lsp::Position position;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextDocumentPositionParams, textDocument, position)

    /**
     * @brief LSP data type
     * 
     */
    struct ReferenceContext
    {
        bool includeDeclaration;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReferenceContext, includeDeclaration)

    /**
     * @brief LSP data type
     * 
     */
    struct ReferenceParams
    {
        lsp::TextDocumentIdentifier textDocument;
        lsp::Position position;
        lsp::ReferenceContext context;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReferenceParams, textDocument, position, context)

    /**
     * @brief LSP data type
     * 
     */
    struct DocumentColorParams
    {
        lsp::TextDocumentIdentifier textDocument;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentColorParams, textDocument)

    /**
     * @brief LSP data type
     * 
     */
    struct ConfigurationItem
    {
        std::string section;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigurationItem, section)

    /**
     * @brief LSP data type
     * 
     */
    struct ConfigurationParams
    {
        std::vector<ConfigurationItem> items;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigurationParams, items)
}
#endif /* TYPES_H */