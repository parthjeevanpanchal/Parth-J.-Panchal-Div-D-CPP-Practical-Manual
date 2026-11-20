#include "InventoryManager.hpp"
#include <algorithm>
#include <numeric>
#include <cctype>

InventoryManager::InventoryManager() : m_nextTxId(1) {}

void InventoryManager::addItem(std::shared_ptr<Item> item) {
    if (!item) {
        throw InvalidInputException("Cannot add null item pointer.");
    }
    int id = item->getId();
    if (m_items.find(id) != m_items.end()) {
        throw DuplicateItemException(id);
    }

    m_items[id] = item;

    // Log initial creation transaction
    Transaction tx(m_nextTxId++, Transaction::getCurrentTimestamp(),
                    id, item->getName(), TransactionType::INITIAL_STOCK,
                    item->getQuantity(), item->getQuantity(), "Item initial creation");
    m_transactions.push_back(tx);
}

void InventoryManager::removeItem(int id) {
    auto it = m_items.find(id);
    if (it == m_items.end()) {
        throw ItemNotFoundException(id);
    }
    m_items.erase(it);
}

std::shared_ptr<Item> InventoryManager::getItem(int id) const {
    auto it = m_items.find(id);
    if (it == m_items.end()) {
        throw ItemNotFoundException(id);
    }
    return it->second;
}

bool InventoryManager::hasItem(int id) const {
    return m_items.find(id) != m_items.end();
}

void InventoryManager::performStockIn(int id, int quantity, const std::string& notes) {
    auto item = getItem(id); // Throws ItemNotFoundException if not found
    item->stockIn(quantity); // Throws InvalidInputException if quantity <= 0

    std::string remark = notes.empty() ? "Stock-in replenishment" : notes;
    Transaction tx(m_nextTxId++, Transaction::getCurrentTimestamp(),
                    id, item->getName(), TransactionType::STOCK_IN,
                    quantity, item->getQuantity(), remark);
    m_transactions.push_back(tx);
}

void InventoryManager::performStockOut(int id, int quantity, const std::string& notes) {
    auto item = getItem(id); // Throws ItemNotFoundException if not found
    // Item::stockOut throws InsufficientStockException if quantity > available
    item->stockOut(quantity);

    std::string remark = notes.empty() ? "Stock-out dispatch" : notes;
    Transaction tx(m_nextTxId++, Transaction::getCurrentTimestamp(),
                    id, item->getName(), TransactionType::STOCK_OUT,
                    -quantity, item->getQuantity(), remark);
    m_transactions.push_back(tx);
}

void InventoryManager::updateItemPrice(int id, double newPrice) {
    auto item = getItem(id);
    item->setPrice(newPrice);

    Transaction tx(m_nextTxId++, Transaction::getCurrentTimestamp(),
                    id, item->getName(), TransactionType::PRICE_UPDATE,
                    0, item->getQuantity(), "Price updated to Rs. " + std::to_string(newPrice));
    m_transactions.push_back(tx);
}

void InventoryManager::updateItemName(int id, const std::string& newName) {
    auto item = getItem(id);
    item->setName(newName);
}

static std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lower;
}

std::vector<std::shared_ptr<Item>> InventoryManager::searchByName(const std::string& keyword) const {
    std::vector<std::shared_ptr<Item>> results;
    std::string lowerKeyword = toLower(keyword);

    for (const auto& pair : m_items) {
        std::string lowerName = toLower(pair.second->getName());
        std::string lowerCat  = toLower(pair.second->getCategory());
        if (lowerName.find(lowerKeyword) != std::string::npos ||
            lowerCat.find(lowerKeyword) != std::string::npos) {
            results.push_back(pair.second);
        }
    }
    return results;
}

std::vector<std::shared_ptr<Item>> InventoryManager::getLowStockItems(int threshold) const {
    std::vector<std::shared_ptr<Item>> lowStockList;
    for (const auto& pair : m_items) {
        if (pair.second->getQuantity() <= threshold) {
            lowStockList.push_back(pair.second);
        }
    }
    return lowStockList;
}

std::vector<std::shared_ptr<Item>> InventoryManager::getSortedItems(SortCriteria criteria, bool ascending) const {
    std::vector<std::shared_ptr<Item>> sortedList;
    sortedList.reserve(m_items.size());
    for (const auto& pair : m_items) {
        sortedList.push_back(pair.second);
    }

    std::sort(sortedList.begin(), sortedList.end(),
        [criteria, ascending](const std::shared_ptr<Item>& a, const std::shared_ptr<Item>& b) {
            bool result = false;
            switch (criteria) {
                case SortCriteria::ID:
                    result = a->getId() < b->getId();
                    break;
                case SortCriteria::NAME:
                    result = a->getName() < b->getName();
                    break;
                case SortCriteria::PRICE:
                    result = a->getPrice() < b->getPrice();
                    break;
                case SortCriteria::QUANTITY:
                    result = a->getQuantity() < b->getQuantity();
                    break;
            }
            return ascending ? result : !result;
        });

    return sortedList;
}

double InventoryManager::getTotalValuation() const {
    return std::accumulate(m_items.begin(), m_items.end(), 0.0,
        [](double sum, const std::pair<const int, std::shared_ptr<Item>>& pair) {
            return sum + pair.second->getValuation();
        });
}

int InventoryManager::getTotalItemCount() const {
    return static_cast<int>(m_items.size());
}

int InventoryManager::getTotalUnits() const {
    return std::accumulate(m_items.begin(), m_items.end(), 0,
        [](int sum, const std::pair<const int, std::shared_ptr<Item>>& pair) {
            return sum + pair.second->getQuantity();
        });
}

void InventoryManager::saveToFiles(const std::string& invFilename, const std::string& txFilename) const {
    FileStorage::saveInventory(invFilename, m_items);
    FileStorage::saveTransactions(txFilename, m_transactions);
}

void InventoryManager::loadFromFiles(const std::string& invFilename, const std::string& txFilename) {
    m_items.clear();
    m_transactions.clear();

    auto loadedItems = FileStorage::loadInventory(invFilename);
    for (const auto& item : loadedItems) {
        m_items[item->getId()] = item;
    }

    m_transactions = FileStorage::loadTransactions(txFilename);

    // Update next transaction ID counter
    int maxId = 0;
    for (const auto& tx : m_transactions) {
        if (tx.getTransactionId() > maxId) {
            maxId = tx.getTransactionId();
        }
    }
    m_nextTxId = maxId + 1;
}

void InventoryManager::clear() {
    m_items.clear();
    m_transactions.clear();
    m_nextTxId = 1;
}
