#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include "block.h"
#include "transaction.h"

class Blockchain {
private:
    std::vector<Block> chain;
    std::vector<Transaction> mempool;
    int difficulty;

    Block createGenesisBlock();
    Block getLatestBlock();
    bool  isTransactionValid(const Transaction& tx);
    bool  txidExists(const std::string& txid) const;
    double getEffectiveBalance(const std::string& address) const;

public:
    Blockchain();

    bool   addTransaction(const Transaction& tx);
    void   minePendingTransactions(std::string minerAddress);
    double getBalance(std::string address);
    std::vector<Transaction> getAllTransactions();
    bool   isChainValid();
    void   saveToFile();
    void   loadFromFile();
};
#endif