#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

/**
 * @brief Base exception class for all inventory-related errors.
 * Inherits from std::runtime_error to leverage standard exception mechanisms.
 */
class InventoryException : public std::runtime_error {
public:
    explicit InventoryException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Exception thrown when an item ID already exists in the inventory.
 */
class DuplicateItemException : public InventoryException {
public:
    explicit DuplicateItemException(int id)
        : InventoryException("Error: Item with ID " + std::to_string(id) + " already exists in inventory.") {}
};

/**
 * @brief Exception thrown when an item ID is not found in the inventory.
 */
class ItemNotFoundException : public InventoryException {
public:
    explicit ItemNotFoundException(int id)
        : InventoryException("Error: Item with ID " + std::to_string(id) + " not found.") {}
};

/**
 * @brief Exception thrown when a stock-out operation exceeds available quantity.
 * Directly satisfies the requirement: "prevent negative stock quantities".
 */
class InsufficientStockException : public InventoryException {
public:
    InsufficientStockException(int id, int requested, int available)
        : InventoryException("Error: Insufficient stock for Item ID " + std::to_string(id) +
                             ". Requested: " + std::to_string(requested) +
                             ", Available: " + std::to_string(available) +
                             ". Operation aborted to prevent negative stock.") {}
};

/**
 * @brief Exception thrown when invalid input or domain invariant violation occurs.
 */
class InvalidInputException : public InventoryException {
public:
    explicit InvalidInputException(const std::string& reason)
        : InventoryException("Validation Error: " + reason) {}
};

/**
 * @brief Exception thrown when reading from or writing to files fails.
 */
class FileStorageException : public InventoryException {
public:
    explicit FileStorageException(const std::string& filename, const std::string& reason)
        : InventoryException("File I/O Error on '" + filename + "': " + reason) {}
};

#endif // EXCEPTIONS_HPP
