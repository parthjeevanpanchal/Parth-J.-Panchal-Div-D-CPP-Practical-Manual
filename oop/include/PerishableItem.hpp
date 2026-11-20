#ifndef PERISHABLE_ITEM_HPP
#define PERISHABLE_ITEM_HPP

#include "Item.hpp"

/**
 * @brief Represents perishable goods (e.g., dairy, produce, pharmaceuticals).
 * Demonstrates Inheritance from Item, adding expiration date and temperature requirements.
 */
class PerishableItem : public Item {
private:
    std::string m_expiryDate; // Format: YYYY-MM-DD
    double m_storageTemp;     // Recommended storage temperature in Celsius

public:
    PerishableItem(int id, const std::string& name, int quantity, double price,
                   const std::string& category, const std::string& expiryDate, double storageTemp);

    std::string getExpiryDate() const { return m_expiryDate; }
    double getStorageTemp() const { return m_storageTemp; }

    void setExpiryDate(const std::string& date);
    void setStorageTemp(double temp) { m_storageTemp = temp; }

    // Polymorphic overrides
    std::string getItemType() const override;
    std::string serializeCSV() const override;
    std::string getDetails() const override;
};

#endif // PERISHABLE_ITEM_HPP
