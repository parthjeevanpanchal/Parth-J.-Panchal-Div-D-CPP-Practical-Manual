#ifndef FILE_STORAGE_HPP
#define FILE_STORAGE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Item.hpp"
#include "PerishableItem.hpp"
#include "ElectronicsItem.hpp"
#include "Transaction.hpp"
#include "Exceptions.hpp"

/**
 * @brief Handles file persistence for inventory items and transaction records.
 * Provides robust CSV reading, writing, and format validation.
 */
class FileStorage {
public:
    /**
     * @brief Parses a single CSV line respecting quotes.
     */
    static std::vector<std::string> parseCSVLine(const std::string& line);

    /**
     * @brief Saves all inventory items to a CSV file.
     * Throws FileStorageException if the file cannot be opened.
     */
    static void saveInventory(const std::string& filename,
                              const std::map<int, std::shared_ptr<Item>>& items);

    /**
     * @brief Loads inventory items from a CSV file.
     * Polymorphically reconstructs Standard, Perishable, and Electronics items.
     * Throws FileStorageException if the file cannot be opened or parsed.
     */
    static std::vector<std::shared_ptr<Item>> loadInventory(const std::string& filename);

    /**
     * @brief Saves transaction audit logs to a CSV file.
     */
    static void saveTransactions(const std::string& filename,
                                 const std::vector<Transaction>& transactions);

    /**
     * @brief Loads transaction audit logs from a CSV file.
     */
    static std::vector<Transaction> loadTransactions(const std::string& filename);
};

#endif // FILE_STORAGE_HPP
