#include "catch.hpp"
#include "xmlParser.hpp"
#include "types.hpp"
#include <iostream>
#include "lspExceptions.hpp"


TEST_CASE("XML Parser")
{
    std::cout << "Enter path to arxml file with %3A instead of the colon (':'):";
    std::string filePath;
    std::cin >> filePath;

    std::shared_ptr<xmlParser> testParser = std::make_shared<xmlParser>();
    lsp::TextDocumentPositionParams testParams;
    testParams.textDocument.uri = "file///" + filePath;
    testParams.position.character = 0;

    testParams.position.line = 0;
    REQUIRE_THROWS_AS(testParser->getDefinition(testParams), lsp::elementNotFoundException);
    testParams.position.line = 100;
    REQUIRE_THROWS_AS(testParser->getDefinition(testParams), lsp::elementNotFoundException);
    testParams.position.character = 100;
    REQUIRE_THROWS_AS(testParser->getDefinition(testParams), lsp::elementNotFoundException);
}