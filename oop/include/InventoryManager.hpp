#ifndef INVENTORY_MANAGER_HPP
#define INVENTORY_MANAGER_HPP

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "Item.hpp"
#include "Transaction.hpp"
#include "Exceptions.hpp"
#include "FileStorage.hpp"

enum class SortCriteria {
    ID,
    NAME,
    PRICE,
    QUANTITY
};

/**
 * @brief Core business logic manager for the inventory system.
 * Leverages STL containers (map, vector) and STL algorithms.
 */
class InventoryManager {
private:
    std::map<int, std::shared_ptr<Item>> m_items;
    std::vector<Transaction> m_transactions;
    int m_nextTxId;

public:
    InventoryManager();

    // Item Management
    void addItem(std::shared_ptr<Item> item);
    void removeItem(int id);
    std::shared_ptr<Item> getItem(int id) const;
    bool hasItem(int id) const;

    // Stock Operations (with negative stock prevention)
    void performStockIn(int id, int quantity, const std::string& notes = "");
    void performStockOut(int id, int quantity, const std::string& notes = "");

    // Update Operations
    void updateItemPrice(int id, double newPrice);
    void updateItemName(int id, const std::string& newName);

    // Queries & Reports using STL
    std::vector<std::shared_ptr<Item>> searchByName(const std::string& keyword) const;
    std::vector<std::shared_ptr<Item>> getLowStockItems(int threshold = 5) const;
    std::vector<std::shared_ptr<Item>> getSortedItems(SortCriteria criteria, bool ascending = true) const;

    double getTotalValuation() const;
    int getTotalItemCount() const;
    int getTotalUnits() const;

    const std::map<int, std::shared_ptr<Item>>& getItems() const { return m_items; }
    const std::vector<Transaction>& getTransactions() const { return m_transactions; }

    // File Persistence
    void saveToFiles(const std::string& invFilename, const std::string& txFilename) const;
    void loadFromFiles(const std::string& invFilename, const std::string& txFilename);

    void clear();
};

#endif // INVENTORY_MANAGER_HPP
