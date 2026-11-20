#ifndef ITEM_HPP
#define ITEM_HPP

#include <string>
#include <iostream>
#include <iomanip>
#include "Exceptions.hpp"

/**
 * @brief Base class representing a generic inventory item.
 * Demonstrates Encapsulation, Constructors, and Polymorphism.
 */
class Item {
protected:
    int m_id;
    std::string m_name;
    int m_quantity;
    double m_price;
    std::string m_category;

public:
    /**
     * @brief Parameterized constructor with validation.
     * Throws InvalidInputException on negative price/quantity or blank name.
     */
    Item(int id, const std::string& name, int quantity, double price, const std::string& category = "General");

    virtual ~Item() = default;

    // Getters (Encapsulation)
    int getId() const { return m_id; }
    std::string getName() const { return m_name; }
    int getQuantity() const { return m_quantity; }
    double getPrice() const { return m_price; }
    std::string getCategory() const { return m_category; }

    // Setters with validation
    void setName(const std::string& newName);
    void setPrice(double newPrice);
    void setCategory(const std::string& newCategory);

    // Business Logic
    virtual void stockIn(int amount);
    virtual void stockOut(int amount);
    virtual double getValuation() const;

    // Polymorphic operations
    virtual std::string getItemType() const;
    virtual void display(std::ostream& os) const;
    virtual std::string serializeCSV() const;
    virtual std::string getDetails() const;
};

#endif // ITEM_HPP
