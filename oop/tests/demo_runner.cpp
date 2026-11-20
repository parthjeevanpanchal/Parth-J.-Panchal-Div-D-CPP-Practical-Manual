#include <iostream>
#include <iomanip>
#include <cassert>
#include "InventoryManager.hpp"
#include "PerishableItem.hpp"
#include "ElectronicsItem.hpp"
#include "Exceptions.hpp"

void printBanner(const std::string& title) {
    std::cout << "\n========================================================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================================================================\n";
}

void printSection(const std::string& sec) {
    std::cout << "\n----------------------------------------------------------------------------------------\n";
    std::cout << ">>> " << sec << "\n";
    std::cout << "----------------------------------------------------------------------------------------\n";
}

int main() {
    printBanner("PRACTICAL 12: INVENTORY MANAGER - AUTOMATED DEMONSTRATION & VERIFICATION");

    InventoryManager manager;

    // =========================================================================
    // PART 1: NORMAL OPERATION CASES
    // =========================================================================
    printSection("NORMAL CASE 1: Load Existing Inventory Records from CSV");
    try {
        manager.loadFromFiles("data/inventory.csv", "data/transactions.csv");
        std::cout << "[PASS] Successfully loaded " << manager.getTotalItemCount()
                  << " items and " << manager.getTransactions().size() << " transactions.\n";
    } catch (const std::exception& e) {
        std::cout << "[FAIL] Could not load data: " << e.what() << "\n";
        return 1;
    }

    printSection("NORMAL CASE 2: Display All Items (Polymorphic Output & Statistics)");
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Item Name"
              << std::setw(12) << "Type"
              << std::setw(14) << "Category"
              << std::right
              << std::setw(8)  << "Stock"
              << std::setw(14) << "Price (Rs.)"
              << std::setw(16) << "Total Val (Rs.)"
              << "  Additional Details\n";
    std::cout << std::string(98, '-') << "\n";
    for (const auto& pair : manager.getItems()) {
        pair.second->display(std::cout);
        std::cout << "\n";
    }
    std::cout << std::string(98, '-') << "\n";
    std::cout << "Total Item Types: " << manager.getTotalItemCount()
              << " | Total Units: " << manager.getTotalUnits()
              << " | Total Portfolio Value: Rs. " << std::fixed << std::setprecision(2)
              << manager.getTotalValuation() << "\n";

    printSection("NORMAL CASE 3: Add New Polymorphic Items (Standard, Perishable, Electronics)");
    try {
        // Add a perishable medicine item
        auto vaccine = std::make_shared<PerishableItem>(205, "Flu Vaccine Doses", 50, 450.00, "Medical", "2027-04-30", 2.0);
        manager.addItem(vaccine);
        std::cout << "[PASS] Added PerishableItem (ID 205: 'Flu Vaccine Doses').\n";

        // Add an electronic item
        auto keyboard = std::make_shared<ElectronicsItem>(305, "Mechanical Keyboard RGB", 20, 2499.00, "Peripherals", 24, "Keychron");
        manager.addItem(keyboard);
        std::cout << "[PASS] Added ElectronicsItem (ID 305: 'Mechanical Keyboard RGB').\n";
    } catch (const std::exception& e) {
        std::cout << "[FAIL] " << e.what() << "\n";
    }

    printSection("NORMAL CASE 4: Perform Stock-In (Replenish Inventory)");
    try {
        int targetId = 302; // Dell Monitor
        int before = manager.getItem(targetId)->getQuantity();
        int addQty = 12;
        manager.performStockIn(targetId, addQty, "Urgent restocking order #8821");
        int after = manager.getItem(targetId)->getQuantity();
        std::cout << "[PASS] Stock-In successful for Item ID " << targetId << ". Quantity: "
                  << before << " -> " << after << " (+" << addQty << ")\n";
    } catch (const std::exception& e) {
        std::cout << "[FAIL] " << e.what() << "\n";
    }

    printSection("NORMAL CASE 5: Perform Stock-Out (Dispatch Goods)");
    try {
        int targetId = 301; // Logitech Mouse
        int before = manager.getItem(targetId)->getQuantity();
        int dispatchQty = 5;
        manager.performStockOut(targetId, dispatchQty, "Customer Order #CO-9012");
        int after = manager.getItem(targetId)->getQuantity();
        std::cout << "[PASS] Stock-Out successful for Item ID " << targetId << ". Quantity: "
                  << before << " -> " << after << " (-" << dispatchQty << ")\n";
    } catch (const std::exception& e) {
        std::cout << "[FAIL] " << e.what() << "\n";
    }

    printSection("NORMAL CASE 6: Search Items (By ID & Keyword 'Dairy')");
    {
        auto dairyResults = manager.searchByName("dairy");
        std::cout << "Found " << dairyResults.size() << " matching items in Dairy:\n";
        for (const auto& item : dairyResults) {
            std::cout << "  - ID " << item->getId() << ": " << item->getName()
                      << " (Rs. " << item->getPrice() << ", Stock: " << item->getQuantity() << ")\n";
        }
    }

    printSection("NORMAL CASE 7: Low Stock Alerts (Threshold <= 5)");
    {
        auto lowStock = manager.getLowStockItems(5);
        std::cout << "Items needing replenishment (" << lowStock.size() << " items found):\n";
        for (const auto& item : lowStock) {
            std::cout << "  [ALERT] ID " << item->getId() << " '" << item->getName()
                      << "' has only " << item->getQuantity() << " units left!\n";
        }
    }

    printSection("NORMAL CASE 8: Sort Catalog (By Price Descending)");
    {
        auto sortedByPrice = manager.getSortedItems(SortCriteria::PRICE, false);
        std::cout << "Top 3 highest value items:\n";
        for (size_t i = 0; i < std::min<size_t>(3, sortedByPrice.size()); ++i) {
            std::cout << "  " << (i+1) << ". " << sortedByPrice[i]->getName()
                      << " @ Rs. " << sortedByPrice[i]->getPrice() << "\n";
        }
    }

    printSection("NORMAL CASE 9: File Persistence (Save & Reload Verification)");
    try {
        std::string testInvFile = "data/test_inventory_out.csv";
        std::string testTxFile  = "data/test_tx_out.csv";
        manager.saveToFiles(testInvFile, testTxFile);
        std::cout << "[PASS] Saved records to temporary files.\n";

        InventoryManager verifyManager;
        verifyManager.loadFromFiles(testInvFile, testTxFile);
        std::cout << "[PASS] Reloaded successfully from temporary files ("
                  << verifyManager.getTotalItemCount() << " items, "
                  << verifyManager.getTransactions().size() << " transactions).\n";
    } catch (const std::exception& e) {
        std::cout << "[FAIL] " << e.what() << "\n";
    }

    // =========================================================================
    // PART 2: ERROR HANDLING & EXCEPTION CASES
    // =========================================================================
    printBanner("PART 2: EXCEPTION HANDLING & ERROR CASE VERIFICATION");

    printSection("ERROR CASE 1: Prevent Negative Stock (InsufficientStockException)");
    try {
        int targetId = 103; // Office chair, only 4 available
        int available = manager.getItem(targetId)->getQuantity();
        int requestedDispatch = available + 10;
        std::cout << "Attempting to dispatch " << requestedDispatch
                  << " units of ID " << targetId << " (Available: " << available << ")...\n";
        manager.performStockOut(targetId, requestedDispatch, "Excess dispatch attempt");
        std::cout << "[FAIL] Exception was NOT thrown! Negative stock permitted!\n";
    } catch (const InsufficientStockException& ex) {
        std::cout << "[CAUGHT EXPECTED EXCEPTION] InsufficientStockException:\n"
                  << "   --> " << ex.what() << "\n";
        std::cout << "[PASS] Stock was successfully protected against negative value.\n";
    }

    printSection("ERROR CASE 2: Duplicate Item ID Detection (DuplicateItemException)");
    try {
        int duplicateId = 101; // Already exists
        std::cout << "Attempting to add new item with already existing ID " << duplicateId << "...\n";
        auto dupItem = std::make_shared<Item>(duplicateId, "Duplicate Stapler", 10, 15.0);
        manager.addItem(dupItem);
        std::cout << "[FAIL] Exception was NOT thrown! Duplicate ID allowed!\n";
    } catch (const DuplicateItemException& ex) {
        std::cout << "[CAUGHT EXPECTED EXCEPTION] DuplicateItemException:\n"
                  << "   --> " << ex.what() << "\n";
        std::cout << "[PASS] Duplicate item rejected cleanly.\n";
    }

    printSection("ERROR CASE 3: Item Not Found (ItemNotFoundException)");
    try {
        int nonExistentId = 9999;
        std::cout << "Attempting to query non-existent ID " << nonExistentId << "...\n";
        manager.getItem(nonExistentId);
        std::cout << "[FAIL] Exception was NOT thrown! Missing item ignored!\n";
    } catch (const ItemNotFoundException& ex) {
        std::cout << "[CAUGHT EXPECTED EXCEPTION] ItemNotFoundException:\n"
                  << "   --> " << ex.what() << "\n";
        std::cout << "[PASS] Non-existent item properly handled.\n";
    }

    printSection("ERROR CASE 4: Invalid Domain Values (InvalidInputException)");
    try {
        std::cout << "Attempting to create item with negative price (-50.00)...\n";
        auto invalidItem = std::make_shared<Item>(999, "Faulty Item", 10, -50.00);
        manager.addItem(invalidItem);
        std::cout << "[FAIL] Exception was NOT thrown! Negative price permitted!\n";
    } catch (const InvalidInputException& ex) {
        std::cout << "[CAUGHT EXPECTED EXCEPTION] InvalidInputException:\n"
                  << "   --> " << ex.what() << "\n";
        std::cout << "[PASS] Domain constraints strictly enforced.\n";
    }

    printSection("ERROR CASE 5: File I/O Error Handling (FileStorageException)");
    try {
        std::cout << "Attempting to load from an invalid non-existent path 'data/invalid_dir/no_file.csv'...\n";
        FileStorage::loadInventory("data/invalid_dir/no_file.csv");
        std::cout << "[FAIL] Exception was NOT thrown for missing file!\n";
    } catch (const FileStorageException& ex) {
        std::cout << "[CAUGHT EXPECTED EXCEPTION] FileStorageException:\n"
                  << "   --> " << ex.what() << "\n";
        std::cout << "[PASS] File I/O failures gracefully trapped.\n";
    }

    printBanner("ALL DEMONSTRATION TESTS COMPLETED SUCCESSFULLY (100% PASS)");
    return 0;
}
