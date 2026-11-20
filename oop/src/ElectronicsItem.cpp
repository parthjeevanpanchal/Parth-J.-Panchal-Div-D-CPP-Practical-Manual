#include "ElectronicsItem.hpp"
#include <sstream>

ElectronicsItem::ElectronicsItem(int id, const std::string& name, int quantity, double price,
                                 const std::string& category, int warrantyMonths, const std::string& brand)
    : Item(id, name, quantity, price, category),
      m_warrantyMonths(warrantyMonths),
      m_brand(brand) {
    if (m_warrantyMonths < 0) {
        throw InvalidInputException("Warranty months cannot be negative.");
    }
    if (m_brand.empty()) {
        m_brand = "Generic";
    }
}

void ElectronicsItem::setWarrantyMonths(int months) {
    if (months < 0) {
        throw InvalidInputException("Warranty months cannot be negative.");
    }
    m_warrantyMonths = months;
}

void ElectronicsItem::setBrand(const std::string& brand) {
    if (brand.empty()) {
        m_brand = "Generic";
    } else {
        m_brand = brand;
    }
}

std::string ElectronicsItem::getItemType() const {
    return "Electronics";
}

std::string ElectronicsItem::serializeCSV() const {
    // Format: Type,ID,Name,Quantity,Price,Category,Extra1,Extra2
    std::ostringstream ss;
    ss << getItemType() << ","
       << m_id << ","
       << "\"" << m_name << "\","
       << m_quantity << ","
       << std::fixed << std::setprecision(2) << m_price << ","
       << "\"" << m_category << "\","
       << m_warrantyMonths << ","
       << "\"" << m_brand << "\"";
    return ss.str();
}

std::string ElectronicsItem::getDetails() const {
    std::ostringstream ss;
    ss << "Warranty: " << m_warrantyMonths << " mos | Brand: " << m_brand;
    return ss.str();
}
