/*
 * Nodevia (NDV) Blockchain System
 *
 * Copyright (c) 2026 Akshaj Chiguru
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */
#include "blockchain.h"
#include "utils.h"
#include "pow.h"
#include "config.h"
#include "wallet.h"
#include <iostream>
#include <fstream>
#include <sstream>

Blockchain::Blockchain() {
    difficulty = 2;
    loadFromFile();
    if (chain.empty()) {
        chain.push_back(createGenesisBlock());
    } else if (!isChainValid()) {
        std::cout << "[ERROR] Blockchain corrupted! Chain validation failed.\n";
    }
    std::cout << "NDV Blockchain started\n";
}

Block Blockchain::createGenesisBlock() {
    // Create genesis block normally
    Block genesis(0, {Transaction("network", "genesis", 100, 0.0)}, "0");

    // Append the fixed message and recalculate hash
    const std::string message = "The Times 2026 — Nodevia begins";
    std::string originalHash = genesis.hash;
    std::string newHash = sha256(originalHash + message);

    // Update the hash with the embedded message
    genesis.hash = newHash;

    return genesis;
}

Block Blockchain::getLatestBlock() { return chain.back(); }

// ── Chain validation ──────────────────────────────────────────────────────────

bool Blockchain::isChainValid() {
    for (size_t i = 0; i < chain.size(); ++i) {
        if (chain[i].calculateHash() != chain[i].hash) {
            std::cout << "[WARN] Block " << i << " hash mismatch\n";
            return false;
        }
        if (i > 0 && chain[i].prevHash != chain[i-1].hash) {
            std::cout << "[WARN] Block " << i << " broken chain link\n";
            return false;
        }
    }
    return true;
}

// ── Duplicate / balance helpers ───────────────────────────────────────────────

bool Blockchain::txidExists(const std::string& txid) const {
    for (auto& tx : mempool)       if (tx.txid == txid) return true;
    for (auto& b  : chain)
        for (auto& tx : b.transactions) if (tx.txid == txid) return true;
    return false;
}

double Blockchain::getEffectiveBalance(const std::string& addr) const {
    double bal = 0;
    for (auto& b : chain)
        for (auto& tx : b.transactions) {
            if (tx.sender   == addr) bal -= (tx.amount + tx.fee);
            if (tx.receiver == addr) bal += tx.amount;
        }
    for (auto& tx : mempool)
        if (tx.sender == addr) bal -= (tx.amount + tx.fee);
    return bal;
}

// ── Transaction management ────────────────────────────────────────────────────

bool Blockchain::addTransaction(const Transaction& tx) {
    if (isTransactionValid(tx)) {
        mempool.push_back(tx);
        std::cout << "TX added [txid: " << tx.txid.substr(0,12) << "...]\n";
        return true;
    }
    return false;  // rejection reason already printed by isTransactionValid
}

bool Blockchain::isTransactionValid(const Transaction& tx) {
    if (tx.amount <= 0) return false;

    if (tx.sender == "network") return true;  // reward txs bypass all checks

    if (txidExists(tx.txid)) {
        std::cout << "[INVALID] Duplicate transaction\n";
        return false;
    }
    if (tx.signature.empty() || tx.publicKey.empty()) {
        std::cout << "[INVALID] Missing signature or public key\n";
        return false;
    }
    if (!Wallet::verifySignature(tx.signingData(), tx.signature, tx.publicKey)) {
        std::cout << "[INVALID] Signature verification failed\n";
        return false;
    }
    if (getEffectiveBalance(tx.sender) < (tx.amount + tx.fee)) {
        std::cout << "[INVALID] Insufficient balance (including pending txns)\n";
        return false;
    }
    return true;
}

// ── Mining ────────────────────────────────────────────────────────────────────

void Blockchain::minePendingTransactions(std::string minerAddress) {
    double totalFees = 0;
    for (auto& tx : mempool) totalFees += tx.fee;

    Transaction reward("network", minerAddress, MINING_REWARD + totalFees);
    mempool.push_back(reward);
    int txCount = (int)mempool.size();

    Block newBlock(chain.size(), mempool, getLatestBlock().hash);
    ProofOfWork::mine(newBlock, difficulty);

    chain.push_back(newBlock);
    mempool.clear();
    saveToFile();

    Block& b = chain.back();
    std::cout << "\n+----------------------------+\n";
    std::cout << "  Block #" << b.index << " mined!\n";
    std::cout << "  Transactions : " << txCount << "\n";
    std::cout << "  Hash         : " << b.hash.substr(0,20) << "...\n";
    std::cout << "  Reward       : " << std::fixed << std::setprecision(4)
              << (MINING_REWARD + totalFees) << " NDV\n";
    std::cout << "  New Balance  : " << std::fixed << std::setprecision(4)
              << getBalance(minerAddress) << " NDV\n";
    std::cout << "+----------------------------+\n\n";
}

// ── Balances ──────────────────────────────────────────────────────────────────

double Blockchain::getBalance(std::string addr) {
    double bal = 0;
    for (auto& b : chain)
        for (auto& tx : b.transactions) {
            if (tx.sender   == addr) bal -= (tx.amount + tx.fee);
            if (tx.receiver == addr) bal += tx.amount;
        }
    return bal;
}

std::vector<Transaction> Blockchain::getAllTransactions() {
    std::vector<Transaction> all;
    for (auto& b : chain)
        for (auto& t : b.transactions)
            all.push_back(t);
    return all;
}

// ── Persistence ───────────────────────────────────────────────────────────────
// Format per block:
//   BLOCK
//   <index>
//   <timestamp>
//   <nonce>
//   <prevHash>
//   <hash>
//   <tx_count>
//   TX <txid> <sender> <receiver> <amount> <fee> <sig|-> <pubkey|->
//   END

void Blockchain::saveToFile() {
    std::ofstream out("blockchain.dat");
    if (!out.is_open()) { std::cerr << "[ERROR] Cannot write blockchain.dat\n"; return; }

    for (auto& b : chain) {
        out << "BLOCK\n"
            << b.index               << "\n"
            << b.timestamp           << "\n"
            << b.nonce               << "\n"
            << b.prevHash            << "\n"
            << b.hash                << "\n"
            << b.transactions.size() << "\n";
        for (auto& tx : b.transactions) {
            out << "TX "
                << (tx.txid.empty()      ? "-" : tx.txid)      << " "
                << tx.sender             << " "
                << tx.receiver           << " "
                << tx.amount             << " "
                << tx.fee                << " "
                << (tx.signature.empty() ? "-" : tx.signature) << " "
                << (tx.publicKey.empty() ? "-" : tx.publicKey) << "\n";
        }
        out << "END\n";
    }
    out.close();
}

void Blockchain::loadFromFile() {
    std::ifstream in("blockchain.dat");
    if (!in.is_open()) return;

    chain.clear();
    std::string line;

    auto strip = [](std::string& s){ if (!s.empty()&&s.back()=='\r') s.pop_back(); };

    while (std::getline(in, line)) {
        strip(line);
        if (line != "BLOCK") continue;

        std::string s_idx,s_ts,s_nc,s_prev,s_hash,s_cnt;
        if (!std::getline(in,s_idx))  break; strip(s_idx);
        if (!std::getline(in,s_ts))   break; strip(s_ts);
        if (!std::getline(in,s_nc))   break; strip(s_nc);
        if (!std::getline(in,s_prev)) break; strip(s_prev);
        if (!std::getline(in,s_hash)) break; strip(s_hash);
        if (!std::getline(in,s_cnt))  break; strip(s_cnt);

        int  idx  = std::stoi(s_idx);
        long ts   = std::stol(s_ts);
        int  nc   = std::stoi(s_nc);
        int  cnt  = std::stoi(s_cnt);

        std::vector<Transaction> txs;
        for (int t = 0; t < cnt; ++t) {
            if (!std::getline(in, line)) break;
            strip(line);
            if (line.size() < 3 || line.substr(0,3) != "TX ") { --t; continue; }

            std::istringstream ss(line.substr(3));
            std::string txid,sender,receiver,sig,pub;
            double amount=0,fee=0;
            ss >> txid >> sender >> receiver >> amount >> fee >> sig >> pub;

            Transaction tx(sender, receiver, amount, fee);
            tx.txid      = (txid == "-") ? tx.txid : txid;
            tx.signature = (sig  == "-") ? ""      : sig;
            tx.publicKey = (pub  == "-") ? ""      : pub;
            txs.push_back(tx);
        }
        // consume END
        while (std::getline(in,line)) { strip(line); if (line=="END") break; }

        chain.emplace_back(idx, ts, nc, s_prev, s_hash, txs);
    }
    std::cout << "Loaded blocks: " << chain.size() << "\n";
}