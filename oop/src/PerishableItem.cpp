#include "PerishableItem.hpp"
#include <sstream>

PerishableItem::PerishableItem(int id, const std::string& name, int quantity, double price,
                               const std::string& category, const std::string& expiryDate, double storageTemp)
    : Item(id, name, quantity, price, category),
      m_expiryDate(expiryDate),
      m_storageTemp(storageTemp) {
    if (m_expiryDate.empty()) {
        m_expiryDate = "2026-12-31";
    }
}

void PerishableItem::setExpiryDate(const std::string& date) {
    if (date.empty()) {
        throw InvalidInputException("Expiry date cannot be empty.");
    }
    m_expiryDate = date;
}

std::string PerishableItem::getItemType() const {
    return "Perishable";
}

std::string PerishableItem::serializeCSV() const {
    // Format: Type,ID,Name,Quantity,Price,Category,Extra1,Extra2
    std::ostringstream ss;
    ss << getItemType() << ","
       << m_id << ","
       << "\"" << m_name << "\","
       << m_quantity << ","
       << std::fixed << std::setprecision(2) << m_price << ","
       << "\"" << m_category << "\","
       << "\"" << m_expiryDate << "\","
       << std::fixed << std::setprecision(1) << m_storageTemp;
    return ss.str();
}

std::string PerishableItem::getDetails() const {
    std::ostringstream ss;
    ss << "Exp: " << m_expiryDate << " | Temp: " << std::fixed << std::setprecision(1) << m_storageTemp << "C";
    return ss.str();
}
