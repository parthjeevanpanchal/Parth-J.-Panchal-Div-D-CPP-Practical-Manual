#include "ConsoleUI.hpp"
#include <iostream>

int main() {
    try {
        ConsoleUI app("data/inventory.csv", "data/transactions.csv");
        app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal unhandled exception: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
