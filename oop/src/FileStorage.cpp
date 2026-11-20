#include "FileStorage.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<std::string> FileStorage::parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            inQuotes = !inQuotes;
        } else if (ch == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else if (ch == '\r') {
            // ignore carriage returns
            continue;
        } else {
            field += ch;
        }
    }
    fields.push_back(field);
    return fields;
}

void FileStorage::saveInventory(const std::string& filename,
                                const std::map<int, std::shared_ptr<Item>>& items) {
    std::ofstream outFile(filename.c_str());
    if (!outFile.is_open()) {
        throw FileStorageException(filename, "Unable to open file for writing.");
    }

    // CSV Header
    outFile << "Type,ID,Name,Quantity,Price,Category,Extra1,Extra2\n";

    for (const auto& pair : items) {
        outFile << pair.second->serializeCSV() << "\n";
    }

    if (outFile.fail()) {
        throw FileStorageException(filename, "Failed to complete writing inventory data.");
    }
    outFile.close();
}

std::vector<std::shared_ptr<Item>> FileStorage::loadInventory(const std::string& filename) {
    std::ifstream inFile(filename.c_str());
    if (!inFile.is_open()) {
        throw FileStorageException(filename, "File not found or cannot be opened for reading.");
    }

    std::vector<std::shared_ptr<Item>> items;
    std::string line;
    int lineNum = 0;

    while (std::getline(inFile, line)) {
        lineNum++;
        if (line.empty()) continue;

        // Skip header
        if (lineNum == 1 && line.find("Type,ID") != std::string::npos) {
            continue;
        }

        std::vector<std::string> tokens = parseCSVLine(line);
        if (tokens.size() < 6) {
            // Malformed row: skip or warn
            continue;
        }

        try {
            std::string type = tokens[0];
            int id = std::stoi(tokens[1]);
            std::string name = tokens[2];
            int quantity = std::stoi(tokens[3]);
            double price = std::stod(tokens[4]);
            std::string category = tokens[5];

            if (type == "Perishable") {
                std::string expiry = (tokens.size() > 6) ? tokens[6] : "2026-12-31";
                double temp = (tokens.size() > 7 && !tokens[7].empty() && tokens[7] != "N/A")
                              ? std::stod(tokens[7]) : 4.0;
                items.push_back(std::make_shared<PerishableItem>(id, name, quantity, price, category, expiry, temp));
            } else if (type == "Electronics") {
                int warranty = (tokens.size() > 6 && !tokens[6].empty() && tokens[6] != "N/A")
                               ? std::stoi(tokens[6]) : 12;
                std::string brand = (tokens.size() > 7) ? tokens[7] : "Generic";
                items.push_back(std::make_shared<ElectronicsItem>(id, name, quantity, price, category, warranty, brand));
            } else {
                // Standard item
                items.push_back(std::make_shared<Item>(id, name, quantity, price, category));
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Corrupted row at line " << lineNum << " in " << filename
                      << " (" << e.what() << "). Skipping.\n";
        }
    }

    inFile.close();
    return items;
}

void FileStorage::saveTransactions(const std::string& filename,
                                   const std::vector<Transaction>& transactions) {
    std::ofstream outFile(filename.c_str());
    if (!outFile.is_open()) {
        throw FileStorageException(filename, "Unable to open transaction log for writing.");
    }

    outFile << "TxID,Timestamp,ItemID,ItemName,Type,QuantityChange,ResultingStock,Notes\n";

    for (const auto& tx : transactions) {
        outFile << tx.serializeCSV() << "\n";
    }

    if (outFile.fail()) {
        throw FileStorageException(filename, "Failed writing transaction records.");
    }
    outFile.close();
}

std::vector<Transaction> FileStorage::loadTransactions(const std::string& filename) {
    std::ifstream inFile(filename.c_str());
    if (!inFile.is_open()) {
        // Return empty if file does not exist yet
        return {};
    }

    std::vector<Transaction> transactions;
    std::string line;
    int lineNum = 0;

    while (std::getline(inFile, line)) {
        lineNum++;
        if (line.empty()) continue;

        if (lineNum == 1 && line.find("TxID,Timestamp") != std::string::npos) {
            continue;
        }

        std::vector<std::string> tokens = parseCSVLine(line);
        if (tokens.size() < 7) continue;

        try {
            int txId = std::stoi(tokens[0]);
            std::string timestamp = tokens[1];
            int itemId = std::stoi(tokens[2]);
            std::string itemName = tokens[3];
            TransactionType type = Transaction::stringToType(tokens[4]);
            int qtyChange = std::stoi(tokens[5]);
            int resultingStock = std::stoi(tokens[6]);
            std::string notes = (tokens.size() > 7) ? tokens[7] : "";

            transactions.emplace_back(txId, timestamp, itemId, itemName, type, qtyChange, resultingStock, notes);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Corrupted transaction at line " << lineNum << " (" << e.what() << "). Skipping.\n";
        }
    }

    inFile.close();
    return transactions;
}
