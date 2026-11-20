#ifndef CONSOLE_UI_HPP
#define CONSOLE_UI_HPP

#include "InventoryManager.hpp"
#include <string>

/**
 * @brief Interactive text-based user interface for the Inventory Manager.
 * Handles input sanitization, menu display, formatting, and exception presentation.
 */
class ConsoleUI {
private:
    InventoryManager m_manager;
    std::string m_invFile;
    std::string m_txFile;

    void displayMainMenu() const;
    void handleAddItem();
    void handleViewAll() const;
    void handleSearch() const;
    void handleUpdateItem();
    void handleStockIn();
    void handleStockOut();
    void handleLowStockAlerts() const;
    void handleViewTransactions() const;
    void handleSortItems() const;
    void handleSaveData();
    void handleLoadData();

    // Utility methods
    static void printHeader(const std::string& title);
    static void printDivider(char ch = '-', int length = 92);
    static void printTableHeader();
    static void printTransactionTableHeader();

public:
    ConsoleUI(const std::string& invFile = "data/inventory.csv",
              const std::string& txFile = "data/transactions.csv");

    void run();

    // Helper functions for safe input
    static int readInt(const std::string& prompt);
    static double readDouble(const std::string& prompt);
    static std::string readString(const std::string& prompt);
    static void pause();
};

#endif // CONSOLE_UI_HPP
