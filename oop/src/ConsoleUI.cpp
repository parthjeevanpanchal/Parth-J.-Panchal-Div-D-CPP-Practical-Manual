#include "ConsoleUI.hpp"
#include <iostream>
#include <iomanip>
#include <limits>

ConsoleUI::ConsoleUI(const std::string& invFile, const std::string& txFile)
    : m_invFile(invFile), m_txFile(txFile) {
    // Attempt automatic reload from default files
    try {
        m_manager.loadFromFiles(m_invFile, m_txFile);
        std::cout << "[System] Successfully loaded existing inventory records ("
                  << m_manager.getTotalItemCount() << " items).\n";
    } catch (const FileStorageException&) {
        std::cout << "[System] No existing database found. Starting with a fresh catalog.\n";
    }
}

void ConsoleUI::printDivider(char ch, int length) {
    std::cout << std::string(length, ch) << "\n";
}

void ConsoleUI::printHeader(const std::string& title) {
    std::cout << "\n";
    printDivider('=', 95);
    int pad = (95 - static_cast<int>(title.length())) / 2;
    std::cout << std::string(std::max(0, pad), ' ') << title << "\n";
    printDivider('=', 95);
}

void ConsoleUI::printTableHeader() {
    printDivider('-');
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
    printDivider('-');
}

void ConsoleUI::printTransactionTableHeader() {
    printDivider('-');
    std::cout << std::left
              << std::setw(6)  << "TxID"
              << std::setw(21) << "Timestamp"
              << std::setw(8)  << "Item ID"
              << std::setw(20) << "Item Name"
              << std::setw(14) << "Operation"
              << std::right
              << std::setw(10) << "Qty Change"
              << std::setw(10) << "Balance"
              << "  Remarks / Notes\n";
    printDivider('-');
}

int ConsoleUI::readInt(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        } else {
            std::cout << " [!] Invalid input. Please enter an integer value.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

double ConsoleUI::readDouble(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        } else {
            std::cout << " [!] Invalid input. Please enter a valid numerical value.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

std::string ConsoleUI::readString(const std::string& prompt) {
    std::string line;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, line);
        // Trim leading and trailing spaces
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            std::cout << " [!] Input cannot be blank. Please try again.\n";
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        return line.substr(first, (last - first + 1));
    }
}

void ConsoleUI::pause() {
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void ConsoleUI::displayMainMenu() const {
    printHeader("INVENTORY MANAGEMENT SYSTEM (OOP & STL)");
    std::cout << " [1] View All Inventory Items\n"
              << " [2] Add New Item (Standard / Perishable / Electronics)\n"
              << " [3] Search Items (By ID or Name Keyword)\n"
              << " [4] Update Item (Name / Price)\n"
              << " [5] Stock-In Operation (Replenish Stock)\n"
              << " [6] Stock-Out Operation (Dispatch / Prevent Negative Stock)\n"
              << " [7] Low Stock Alerts (Reorder Notifications)\n"
              << " [8] View Transaction Audit Trail\n"
              << " [9] Sort Inventory Catalog\n"
              << " [10] Save Inventory & Transactions to Disk\n"
              << " [11] Reload Records from Disk\n"
              << " [0] Exit Application\n";
    printDivider('-');
}

void ConsoleUI::run() {
    int choice = -1;
    while (choice != 0) {
        displayMainMenu();
        choice = readInt("Select an option [0-11]: ");

        try {
            switch (choice) {
                case 1:  handleViewAll(); break;
                case 2:  handleAddItem(); break;
                case 3:  handleSearch(); break;
                case 4:  handleUpdateItem(); break;
                case 5:  handleStockIn(); break;
                case 6:  handleStockOut(); break;
                case 7:  handleLowStockAlerts(); break;
                case 8:  handleViewTransactions(); break;
                case 9:  handleSortItems(); break;
                case 10: handleSaveData(); break;
                case 11: handleLoadData(); break;
                case 0: {
                    std::string savePrompt = readString("Save changes before exiting? (y/n): ");
                    if (savePrompt == "y" || savePrompt == "Y") {
                        handleSaveData();
                    }
                    std::cout << "\nThank you for using Inventory Management System. Goodbye!\n";
                    break;
                }
                default:
                    std::cout << " [!] Invalid option. Please select between 0 and 11.\n";
                    pause();
                    break;
            }
        } catch (const InventoryException& ex) {
            std::cout << "\n>>> [INVENTORY EXCEPTION CAUGHT] <<<\n"
                      << "    " << ex.what() << "\n";
            pause();
        } catch (const std::exception& ex) {
            std::cout << "\n>>> [STANDARD EXCEPTION CAUGHT] <<<\n"
                      << "    " << ex.what() << "\n";
            pause();
        }
    }
}

void ConsoleUI::handleViewAll() const {
    printHeader("ALL INVENTORY ITEMS");
    const auto& items = m_manager.getItems();
    if (items.empty()) {
        std::cout << " Inventory is currently empty. Add items using Option 2.\n";
        pause();
        return;
    }

    printTableHeader();
    for (const auto& pair : items) {
        pair.second->display(std::cout);
        std::cout << "\n";
    }
    printDivider('-');

    std::cout << std::left
              << "Summary: " << m_manager.getTotalItemCount() << " Unique Item(s) | "
              << m_manager.getTotalUnits() << " Total Unit(s) in Stock | "
              << "Portfolio Valuation: Rs. " << std::fixed << std::setprecision(2)
              << m_manager.getTotalValuation() << "\n";

    pause();
}

void ConsoleUI::handleAddItem() {
    printHeader("ADD NEW INVENTORY ITEM");
    std::cout << "Select Item Category:\n"
              << "  [1] Standard Goods (General merchandise, tools, apparel)\n"
              << "  [2] Perishable Goods (Food, dairy, pharmaceuticals)\n"
              << "  [3] Electronics (Gadgets, hardware, computing)\n";
    int typeChoice = readInt("Enter choice [1-3]: ");

    int id = readInt("Enter Unique Item ID: ");
    if (m_manager.hasItem(id)) {
        throw DuplicateItemException(id);
    }

    std::string name = readString("Enter Item Name: ");
    std::string category = readString("Enter Category: ");
    int quantity = readInt("Enter Initial Stock Quantity: ");
    double price = readDouble("Enter Unit Price (Rs.): ");

    std::shared_ptr<Item> newItem = nullptr;

    if (typeChoice == 2) {
        std::string expiry = readString("Enter Expiration Date (YYYY-MM-DD): ");
        double temp = readDouble("Enter Recommended Storage Temp (C): ");
        newItem = std::make_shared<PerishableItem>(id, name, quantity, price, category, expiry, temp);
    } else if (typeChoice == 3) {
        int warranty = readInt("Enter Warranty Period (in months): ");
        std::string brand = readString("Enter Brand / Manufacturer: ");
        newItem = std::make_shared<ElectronicsItem>(id, name, quantity, price, category, warranty, brand);
    } else {
        newItem = std::make_shared<Item>(id, name, quantity, price, category);
    }

    m_manager.addItem(newItem);
    std::cout << "\n[Success] Item '" << name << "' (ID: " << id << ") successfully added to inventory!\n";
    pause();
}

void ConsoleUI::handleSearch() const {
    printHeader("SEARCH INVENTORY");
    std::cout << "Search By:\n"
              << "  [1] Item ID\n"
              << "  [2] Name / Category Keyword\n";
    int choice = readInt("Enter choice [1-2]: ");

    if (choice == 1) {
        int id = readInt("Enter Item ID to search: ");
        auto item = m_manager.getItem(id); // Throws ItemNotFoundException if missing
        printTableHeader();
        item->display(std::cout);
        std::cout << "\n";
        printDivider('-');
    } else {
        std::string query = readString("Enter search keyword: ");
        auto results = m_manager.searchByName(query);
        if (results.empty()) {
            std::cout << "No matching items found for query '" << query << "'.\n";
        } else {
            printTableHeader();
            for (const auto& item : results) {
                item->display(std::cout);
                std::cout << "\n";
            }
            printDivider('-');
            std::cout << "Found " << results.size() << " matching item(s).\n";
        }
    }
    pause();
}

void ConsoleUI::handleUpdateItem() {
    printHeader("UPDATE ITEM INFORMATION");
    int id = readInt("Enter Item ID to update: ");
    auto item = m_manager.getItem(id); // Throws ItemNotFoundException if not found

    std::cout << "\nCurrent details for Item ID " << id << ":\n";
    printTableHeader();
    item->display(std::cout);
    std::cout << "\n";
    printDivider('-');

    std::cout << "Select field to update:\n"
              << "  [1] Unit Price\n"
              << "  [2] Item Name\n"
              << "  [3] Category\n";
    int fieldChoice = readInt("Enter choice [1-3]: ");

    if (fieldChoice == 1) {
        double newPrice = readDouble("Enter new price (Rs.): ");
        m_manager.updateItemPrice(id, newPrice);
        std::cout << "[Success] Price updated successfully.\n";
    } else if (fieldChoice == 2) {
        std::string newName = readString("Enter new name: ");
        m_manager.updateItemName(id, newName);
        std::cout << "[Success] Name updated successfully.\n";
    } else if (fieldChoice == 3) {
        std::string newCat = readString("Enter new category: ");
        item->setCategory(newCat);
        std::cout << "[Success] Category updated successfully.\n";
    } else {
        std::cout << "[!] Invalid field selection.\n";
    }
    pause();
}

void ConsoleUI::handleStockIn() {
    printHeader("STOCK-IN OPERATION (REPLENISHMENT)");
    int id = readInt("Enter Item ID: ");
    auto item = m_manager.getItem(id);

    std::cout << "Current stock for '" << item->getName() << "' is: " << item->getQuantity() << " units.\n";
    int qty = readInt("Enter quantity to add: ");
    std::string notes = readString("Enter supplier / PO notes: ");

    m_manager.performStockIn(id, qty, notes);
    std::cout << "\n[Success] Added " << qty << " units. New stock balance: "
              << item->getQuantity() << " units.\n";
    pause();
}

void ConsoleUI::handleStockOut() {
    printHeader("STOCK-OUT OPERATION (DISPATCH / SALE)");
    int id = readInt("Enter Item ID: ");
    auto item = m_manager.getItem(id);

    std::cout << "Current stock for '" << item->getName() << "' is: " << item->getQuantity() << " units.\n";
    int qty = readInt("Enter quantity to dispatch: ");
    std::string notes = readString("Enter customer / order notes: ");

    // Will throw InsufficientStockException if qty > item->getQuantity()
    m_manager.performStockOut(id, qty, notes);
    std::cout << "\n[Success] Dispatched " << qty << " units. New stock balance: "
              << item->getQuantity() << " units.\n";
    pause();
}

void ConsoleUI::handleLowStockAlerts() const {
    printHeader("LOW STOCK ALERTS");
    int threshold = readInt("Enter low-stock threshold [Default 5]: ");
    if (threshold < 0) threshold = 5;

    auto lowStock = m_manager.getLowStockItems(threshold);
    if (lowStock.empty()) {
        std::cout << "[OK] All inventory items are adequately stocked above " << threshold << " units.\n";
    } else {
        std::cout << "[WARNING] The following " << lowStock.size() << " item(s) are at or below reorder threshold (" << threshold << "):\n";
        printTableHeader();
        for (const auto& item : lowStock) {
            item->display(std::cout);
            std::cout << "  <-- REORDER NEEDED!\n";
        }
        printDivider('-');
    }
    pause();
}

void ConsoleUI::handleViewTransactions() const {
    printHeader("AUDIT TRAIL / TRANSACTION HISTORY");
    const auto& txList = m_manager.getTransactions();
    if (txList.empty()) {
        std::cout << "No transaction history recorded yet.\n";
        pause();
        return;
    }

    printTransactionTableHeader();
    for (const auto& tx : txList) {
        tx.display(std::cout);
        std::cout << "\n";
    }
    printDivider('-');
    std::cout << "Total Transactions Logged: " << txList.size() << "\n";
    pause();
}

void ConsoleUI::handleSortItems() const {
    printHeader("SORT INVENTORY CATALOG");
    std::cout << "Sort By:\n"
              << "  [1] Item ID\n"
              << "  [2] Name (Alphabetical)\n"
              << "  [3] Unit Price\n"
              << "  [4] Stock Quantity\n";
    int sortChoice = readInt("Select sort criteria [1-4]: ");

    std::cout << "Order:\n"
              << "  [1] Ascending\n"
              << "  [2] Descending\n";
    int orderChoice = readInt("Select order [1-2]: ");
    bool ascending = (orderChoice != 2);

    SortCriteria criteria = SortCriteria::ID;
    if (sortChoice == 2) criteria = SortCriteria::NAME;
    else if (sortChoice == 3) criteria = SortCriteria::PRICE;
    else if (sortChoice == 4) criteria = SortCriteria::QUANTITY;

    auto sortedList = m_manager.getSortedItems(criteria, ascending);
    printTableHeader();
    for (const auto& item : sortedList) {
        item->display(std::cout);
        std::cout << "\n";
    }
    printDivider('-');
    pause();
}

void ConsoleUI::handleSaveData() {
    printHeader("SAVE INVENTORY & TRANSACTION DATA");
    std::cout << "Saving inventory to: " << m_invFile << "\n";
    std::cout << "Saving transactions to: " << m_txFile << "\n";

    m_manager.saveToFiles(m_invFile, m_txFile);
    std::cout << "[Success] All records successfully persisted to CSV files!\n";
    pause();
}

void ConsoleUI::handleLoadData() {
    printHeader("RELOAD INVENTORY FROM DISK");
    std::string confirm = readString("Warning: Reloading will overwrite current in-memory data. Proceed? (y/n): ");
    if (confirm != "y" && confirm != "Y") {
        std::cout << "Operation cancelled.\n";
        pause();
        return;
    }

    m_manager.loadFromFiles(m_invFile, m_txFile);
    std::cout << "[Success] Reloaded " << m_manager.getTotalItemCount()
              << " items and " << m_manager.getTransactions().size()
              << " transactions from disk.\n";
    pause();
}
