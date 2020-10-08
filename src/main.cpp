#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

int main() {
    json j = {
        {
            "banana", "test"
        }
    };

    std::cout << j << std::endl;
}


