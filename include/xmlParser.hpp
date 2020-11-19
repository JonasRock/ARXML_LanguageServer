#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <string>
#include <vector>
#include <list>

#include "boost/iostreams/device/mapped_file.hpp"

#include "types.hpp"
#include "arxmlStorage.hpp"

namespace lsp
{


class XmlParser
{
    struct StorageElement
    {
        std::shared_ptr<lsp::ArxmlStorage> storage;
        uint32_t lastUsedID;
    };

public:
    const lsp::types::Hover getHover(const lsp::types::TextDocumentPositionParams &params);
    const lsp::types::LocationLink getDefinition(const lsp::types::TextDocumentPositionParams &params);
    std::vector<lsp::types::Location> getReferences(const lsp::types::ReferenceParams &params);
    std::vector<lsp::types::non_standard::ShortnameTreeElement> getChildren(const lsp::types::non_standard::GetChildrenParams &params);
    lsp::types::Location getOwner(const lsp::types::non_standard::OwnerParams &params);

    void preParse(const lsp::types::DocumentUri uri);
    void parseFullFolder(const lsp::types::DocumentUri uri);
;

private:
    std::shared_ptr<lsp::ArxmlStorage> getStorageForUri(const lsp::types::DocumentUri uri);
    void parseSingleFile(const std::string sanitizedFilePath, std::shared_ptr<ArxmlStorage> storage);
    void parseNewlines(boost::iostreams::mapped_file &mmap, std::shared_ptr<ArxmlStorage> storage, uint32_t fileIndex);
    void parseShortnamesAndReferences(boost::iostreams::mapped_file &mmap, std::shared_ptr<ArxmlStorage> storage, uint32_t fileIndex);

    std::list<StorageElement> storages_;
};


}

#endif /* XMLPARSER_H */