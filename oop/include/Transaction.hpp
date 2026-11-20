#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <string>
#include <iostream>
#include <iomanip>

enum class TransactionType {
    INITIAL_STOCK,
    STOCK_IN,
    STOCK_OUT,
    PRICE_UPDATE
};

/**
 * @brief Represents an immutable audit trail entry for stock movements.
 */
class Transaction {
private:
    int m_transactionId;
    std::string m_timestamp;
    int m_itemId;
    std::string m_itemName;
    TransactionType m_type;
    int m_quantityChange;
    int m_resultingStock;
    std::string m_notes;

public:
    Transaction(int txId, const std::string& timestamp, int itemId,
                const std::string& itemName, TransactionType type,
                int quantityChange, int resultingStock, const std::string& notes = "");

    int getTransactionId() const { return m_transactionId; }
    std::string getTimestamp() const { return m_timestamp; }
    int getItemId() const { return m_itemId; }
    std::string getItemName() const { return m_itemName; }
    TransactionType getType() const { return m_type; }
    int getQuantityChange() const { return m_quantityChange; }
    int getResultingStock() const { return m_resultingStock; }
    std::string getNotes() const { return m_notes; }

    std::string getTypeString() const;
    void display(std::ostream& os) const;
    std::string serializeCSV() const;

    static std::string typeToString(TransactionType type);
    static TransactionType stringToType(const std::string& typeStr);
    static std::string getCurrentTimestamp();
};

#endif // TRANSACTION_HPP
