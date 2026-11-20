#include "Item.hpp"
#include <sstream>

Item::Item(int id, const std::string& name, int quantity, double price, const std::string& category)
    : m_id(id), m_name(name), m_quantity(quantity), m_price(price), m_category(category) {
    if (m_id <= 0) {
        throw InvalidInputException("Item ID must be a positive integer.");
    }
    if (m_name.empty()) {
        throw InvalidInputException("Item name cannot be empty.");
    }
    if (m_quantity < 0) {
        throw InvalidInputException("Initial quantity cannot be negative.");
    }
    if (m_price < 0.0) {
        throw InvalidInputException("Price cannot be negative.");
    }
    if (m_category.empty()) {
        m_category = "General";
    }
}

void Item::setName(const std::string& newName) {
    if (newName.empty()) {
        throw InvalidInputException("Item name cannot be empty.");
    }
    m_name = newName;
}

void Item::setPrice(double newPrice) {
    if (newPrice < 0.0) {
        throw InvalidInputException("Price cannot be negative.");
    }
    m_price = newPrice;
}

void Item::setCategory(const std::string& newCategory) {
    if (newCategory.empty()) {
        m_category = "General";
    } else {
        m_category = newCategory;
    }
}

void Item::stockIn(int amount) {
    if (amount <= 0) {
        throw InvalidInputException("Stock-in quantity must be strictly greater than zero.");
    }
    m_quantity += amount;
}

void Item::stockOut(int amount) {
    if (amount <= 0) {
        throw InvalidInputException("Stock-out quantity must be strictly greater than zero.");
    }
    if (amount > m_quantity) {
        throw InsufficientStockException(m_id, amount, m_quantity);
    }
    m_quantity -= amount;
}

double Item::getValuation() const {
    return m_quantity * m_price;
}

std::string Item::getItemType() const {
    return "Standard";
}

void Item::display(std::ostream& os) const {
    os << std::left
       << std::setw(6)  << m_id
       << std::setw(22) << (m_name.length() > 20 ? m_name.substr(0, 17) + "..." : m_name)
       << std::setw(12) << getItemType()
       << std::setw(14) << m_category
       << std::right
       << std::setw(8)  << m_quantity
       << std::setw(12) << std::fixed << std::setprecision(2) << m_price
       << std::setw(14) << std::fixed << std::setprecision(2) << getValuation()
       << "  " << getDetails();
}

std::string Item::serializeCSV() const {
    // Format: Type,ID,Name,Quantity,Price,Category,Extra1,Extra2
    std::ostringstream ss;
    ss << getItemType() << ","
       << m_id << ","
       << "\"" << m_name << "\","
       << m_quantity << ","
       << std::fixed << std::setprecision(2) << m_price << ","
       << "\"" << m_category << "\","
       << "N/A,N/A";
    return ss.str();
}

std::string Item::getDetails() const {
    return "-";
}
