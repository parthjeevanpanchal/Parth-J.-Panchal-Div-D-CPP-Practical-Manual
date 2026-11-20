#include "Transaction.hpp"
#include <sstream>
#include <ctime>

Transaction::Transaction(int txId, const std::string& timestamp, int itemId,
                         const std::string& itemName, TransactionType type,
                         int quantityChange, int resultingStock, const std::string& notes)
    : m_transactionId(txId),
      m_timestamp(timestamp),
      m_itemId(itemId),
      m_itemName(itemName),
      m_type(type),
      m_quantityChange(quantityChange),
      m_resultingStock(resultingStock),
      m_notes(notes) {}

std::string Transaction::getTypeString() const {
    return typeToString(m_type);
}

std::string Transaction::typeToString(TransactionType type) {
    switch (type) {
        case TransactionType::INITIAL_STOCK: return "INITIAL";
        case TransactionType::STOCK_IN:      return "STOCK_IN";
        case TransactionType::STOCK_OUT:     return "STOCK_OUT";
        case TransactionType::PRICE_UPDATE:  return "PRICE_UPDATE";
        default:                             return "UNKNOWN";
    }
}

TransactionType Transaction::stringToType(const std::string& typeStr) {
    if (typeStr == "INITIAL" || typeStr == "INITIAL_STOCK") return TransactionType::INITIAL_STOCK;
    if (typeStr == "STOCK_IN")  return TransactionType::STOCK_IN;
    if (typeStr == "STOCK_OUT") return TransactionType::STOCK_OUT;
    if (typeStr == "PRICE_UPDATE") return TransactionType::PRICE_UPDATE;
    return TransactionType::STOCK_IN;
}

std::string Transaction::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    return std::string(buffer);
}

void Transaction::display(std::ostream& os) const {
    os << std::left
       << std::setw(6)  << m_transactionId
       << std::setw(21) << m_timestamp
       << std::setw(8)  << m_itemId
       << std::setw(20) << (m_itemName.length() > 18 ? m_itemName.substr(0, 15) + "..." : m_itemName)
       << std::setw(14) << getTypeString()
       << std::right
       << std::setw(10) << (m_quantityChange > 0 ? "+" + std::to_string(m_quantityChange) : std::to_string(m_quantityChange))
       << std::setw(10) << m_resultingStock
       << "  " << m_notes;
}

std::string Transaction::serializeCSV() const {
    // Format: TxID,Timestamp,ItemID,ItemName,Type,QuantityChange,ResultingStock,Notes
    std::ostringstream ss;
    ss << m_transactionId << ","
       << "\"" << m_timestamp << "\","
       << m_itemId << ","
       << "\"" << m_itemName << "\","
       << getTypeString() << ","
       << m_quantityChange << ","
       << m_resultingStock << ","
       << "\"" << m_notes << "\"";
    return ss.str();
}
