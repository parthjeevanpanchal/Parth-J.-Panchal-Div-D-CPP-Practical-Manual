#ifndef ELECTRONICS_ITEM_HPP
#define ELECTRONICS_ITEM_HPP

#include "Item.hpp"

/**
 * @brief Represents electronic items with warranty and brand attributes.
 * Demonstrates another branch of Inheritance and Polymorphic behavior.
 */
class ElectronicsItem : public Item {
private:
    int m_warrantyMonths;
    std::string m_brand;

public:
    ElectronicsItem(int id, const std::string& name, int quantity, double price,
                    const std::string& category, int warrantyMonths, const std::string& brand);

    int getWarrantyMonths() const { return m_warrantyMonths; }
    std::string getBrand() const { return m_brand; }

    void setWarrantyMonths(int months);
    void setBrand(const std::string& brand);

    // Polymorphic overrides
    std::string getItemType() const override;
    std::string serializeCSV() const override;
    std::string getDetails() const override;
};

#endif // ELECTRONICS_ITEM_HPP
