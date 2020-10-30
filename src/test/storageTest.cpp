#include "catch.hpp"
#include "shortnameStorage.hpp"
#include "lspExceptions.hpp"
#include <utility>

//Testing shortnameStorage

TEST_CASE("shortnameStorage - shortnames")
{
    shortnameStorage storage;
    SECTION("0 Elements")
    {
        CHECK_THROWS_AS( storage.getByFullPath("testPath1/testPath2/testName"), lsp::elementNotFoundException);
        CHECK_THROWS_AS( storage.getByFullPath("/testPath1/testPath2/testName"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("testPath1/testPath2/testName/"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("testPath1/testPath2"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("testPAth1/testPath2/testName"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("testPath2/testName"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath(""), lsp::elementNotFoundException );

        CHECK_THROWS_AS( storage.getByOffset(0), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(500), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(-500), lsp::elementNotFoundException );
    }

    SECTION("1 Element")
    {
        shortnameElement element = { "testPath1/testPath2", "testName", 1000};
        storage.addShortname(element);
        CHECK_NOTHROW( storage.getByFullPath("testPath1/testPath2/testName"));

        SECTION("Wrong values")
        {
        CHECK_THROWS_AS( storage.getByFullPath("/testPath1/testPath2/testName"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("testPath1/testPath2/testName/"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("testPath1/testPath2"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("testPAth1/testPath2/testName"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("testPath2/testName"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath(""), lsp::elementNotFoundException );

        CHECK_THROWS_AS( storage.getByOffset(999), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(1008), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(0), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(-500), lsp::elementNotFoundException );
        }

        SECTION("Correct Range")
        {
            auto i = GENERATE(range(1000, 1007));
            CHECK((i > 999 && i < 1008));
            CHECK_NOTHROW(storage.getByOffset(i));
        }
    }

    SECTION("Multiple Elements")
    {
        shortnameElement element = { "", "test1", 0};
        shortnameElement element1 = { "test1", "subTest1", 50};
        shortnameElement element2 = { "test1", "subTest2", 70};
        shortnameElement element3 = { "test1/subTest1", "sub2Test1", 110};
        shortnameElement element4 = { "", "test2", 130};
        shortnameElement element5 = { "", "test3", 170};
        shortnameElement element6 = { "test3", "subTest1", 190};

        storage.addShortname(element);
        storage.addShortname(element1);
        storage.addShortname(element2);
        storage.addShortname(element3);
        storage.addShortname(element4);
        storage.addShortname(element5);
        storage.addShortname(element6);

        CHECK_NOTHROW( storage.getByFullPath("test1"));
        CHECK_NOTHROW( storage.getByFullPath("test1/subTest1"));
        CHECK_NOTHROW( storage.getByFullPath("test1/subTest2"));
        CHECK_NOTHROW( storage.getByFullPath("test1/subTest1/sub2Test1"));
        CHECK_NOTHROW( storage.getByFullPath("test2"));
        CHECK_NOTHROW( storage.getByFullPath("test3"));
        CHECK_NOTHROW( storage.getByFullPath("test3/subTest1"));

        CHECK_THROWS_AS( storage.getByFullPath("test1/test2"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("test1/subTest3"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("subTest2"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("subTest1/sub2Test1"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("subTest1/test3"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath(""), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("invalid"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("\"\""), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("test1subTest2"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("/subtest1/sub2Test1"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath(" test1"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("test1 "), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("te st1"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("te\nst1"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("\ntest1"), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByFullPath("test1\\subTest2"), lsp::elementNotFoundException );

        CHECK_NOTHROW( storage.getByOffset(0));
        CHECK_NOTHROW( storage.getByOffset(50));
        CHECK_NOTHROW( storage.getByOffset(4));
        CHECK_NOTHROW( storage.getByOffset(197));
        CHECK_NOTHROW( storage.getByOffset(131));
        CHECK_NOTHROW( storage.getByOffset(116));

        CHECK_THROWS_AS( storage.getByOffset(-1), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(5), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(198), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(250), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(-30), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(-169), lsp::elementNotFoundException );
        CHECK_THROWS_AS( storage.getByOffset(-189), lsp::elementNotFoundException );
    }
}

TEST_CASE("shortnameStorage - references")
{
    shortnameStorage storage;
    SECTION("0 References")
    {
        auto i = GENERATE(take(50, random(-500, 1000)));
        CHECK_THROWS_AS(storage.getReferenceByOffset(i), lsp::elementNotFoundException);
    }

    SECTION("1 Reference")
    {
        referenceRange ref{std::make_pair(25, 30), std::make_pair(35, 40)};
        storage.addReference(ref);

        CHECK_NOTHROW(storage.getReferenceByOffset(25));
        CHECK_NOTHROW(storage.getReferenceByOffset(27));
        CHECK_NOTHROW(storage.getReferenceByOffset(30));
        CHECK_THROWS_AS(storage.getReferenceByOffset(24), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(31), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(-3), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(0), lsp::elementNotFoundException);
    }

    SECTION("Multiple References")
    {
        referenceRange ref{std::make_pair(27, 30), std::make_pair(35, 40)};
        referenceRange ref1{std::make_pair(35, 50), std::make_pair(55, 60)};
        referenceRange ref2{std::make_pair(25, 25), std::make_pair(0, 2)};
        referenceRange ref3{std::make_pair(100, 150), std::make_pair(35, 40)};
        storage.addReference(ref);
        storage.addReference(ref1);
        storage.addReference(ref2);
        storage.addReference(ref3);

        CHECK_NOTHROW(storage.getReferenceByOffset(25));
        CHECK_NOTHROW(storage.getReferenceByOffset(30));
        CHECK_NOTHROW(storage.getReferenceByOffset(35));
        CHECK_NOTHROW(storage.getReferenceByOffset(27));
        CHECK_NOTHROW(storage.getReferenceByOffset(150));
        CHECK_NOTHROW(storage.getReferenceByOffset(130));
        CHECK_THROWS_AS(storage.getReferenceByOffset(26), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(31), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(33), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(26), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(24), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(151), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(0), lsp::elementNotFoundException);
        CHECK_THROWS_AS(storage.getReferenceByOffset(-3), lsp::elementNotFoundException);

    }

    SECTION("Malformed References")
    {
        referenceRange ref1{std::make_pair(-3, -12), std::make_pair(10, 20)};
        referenceRange ref2{std::make_pair(20, 10), std::make_pair(10, 20)};
        referenceRange ref3{std::make_pair(10, 20), std::make_pair(45, 5)};

        CHECK_THROWS_AS(storage.addReference(ref1), lsp::malformedElementInsertionException);
        CHECK_THROWS_AS(storage.addReference(ref2), lsp::malformedElementInsertionException);
        CHECK_THROWS_AS(storage.addReference(ref3), lsp::malformedElementInsertionException);
    }
}