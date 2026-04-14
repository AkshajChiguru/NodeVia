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
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "blockchain.h"
#include "wallet.h"
#include "seed.h"

std::vector<Wallet> wallets;

// Shorten "NDV-878349723014" → "NDV-8783...3014" for display only
static std::string shortAddr(const std::string& a) {
    if (a.size() <= 12) return a;
    return a.substr(0,8) + "..." + a.substr(a.size()-4);
}

// ── Wallet persistence ────────────────────────────────────────────────────────
// wallets.dat: one wallet per line
//   <seed> <address> <privateKey> <passwordHash|->

static void saveWallets() {
    std::ofstream out("wallets.dat");
    if (!out.is_open()) { std::cerr << "[ERROR] Cannot open wallets.dat\n"; return; }
    for (auto& w : wallets) {
        std::string ph = w.getPasswordHash().empty() ? "-" : w.getPasswordHash();
        out << w.getSeed()          << " "
            << w.getAddress()       << " "
            << w.getPrivateKeyRaw() << " "   // raw key — never "WRONG_PASSWORD"
            << ph                   << "\n";
    }
    std::cout << "[INFO] Wallets saved (" << wallets.size() << ")\n";
}

static void loadWallets() {
    std::ifstream in("wallets.dat");
    if (!in.is_open()) return;
    wallets.clear();
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back()=='\r') line.pop_back();
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string seed,addr,priv,pass="-";
        if (!(ss >> seed >> addr >> priv)) continue;
        ss >> pass;
        if (pass == "-") pass = "";
        wallets.emplace_back(seed, addr, priv, pass);
    }
    std::cout << "[INFO] Wallets loaded: " << wallets.size() << "\n";
}

static Wallet* findWallet(const std::string& addr) {
    for (auto& w : wallets) if (w.getAddress()==addr) return &w;
    return nullptr;
}

static void printHelp() {
    std::cout << "\n+--------------------------------------------+\n";
    std::cout << "|      Nodevia (NDV) v0.1 - Commands         |\n";
    std::cout << "+--------------------------------------------+\n";
    std::cout << "|  create              Create a new wallet   |\n";
    std::cout << "|  list                List all wallets      |\n";
    std::cout << "|  balance <addr>      Check NDV balance     |\n";
    std::cout << "|  mine <addr>         Mine pending txns     |\n";
    std::cout << "|  send <f> <t> <amt>  Send NDV (signed)    |\n";
    std::cout << "|  history <addr>      Transaction history   |\n";
    std::cout << "|  export <addr>       Show keys + seed      |\n";
    std::cout << "|  passwd <addr>       Change password       |\n";
    std::cout << "|  help                Show this menu        |\n";
    std::cout << "|  exit                Save & quit           |\n";
    std::cout << "+--------------------------------------------+\n\n";
}

int main() {
    Blockchain nodevia;
    loadWallets();

    std::cout << "\n=== Nodevia (NDV) v0.1 ===\n";
    std::cout << "Type 'help' for available commands.\n\n";

    std::string cmd;
    while (true) {
        std::cout << "ndv> ";
        std::cin >> cmd;

        // ── CREATE ───────────────────────────────────────────────
        if (cmd == "create") {
            std::string pass;
            std::cout << "Set password (- to skip): ";
            std::cin >> pass;
            if (pass == "-") pass = "";
            Wallet w;
            if (!pass.empty()) w.setPassword("", pass);
            std::cout << "\nWallet Created\n";
            std::cout << "  Address    : " << w.getAddress()   << "\n";
            std::cout << "  Public Key : " << w.getPublicKey() << "\n";
            std::cout << "  Seed       : " << w.getSeed()      << "  <- SAVE THIS SAFELY\n\n";
            wallets.push_back(w);
            saveWallets();
        }

        // ── LIST ─────────────────────────────────────────────────
        else if (cmd == "list") {
            if (wallets.empty()) { std::cout << "[INFO] No wallets yet.\n"; continue; }
            std::cout << "\n+-----+---------------+----------+--------------+\n";
            std::cout << "|  #  | Address       | Password | Balance      |\n";
            std::cout << "+-----+---------------+----------+--------------+\n";
            int i=1;
            for (auto& w : wallets) {
                std::string lock = w.getPasswordHash().empty() ? "No" : "Yes";
                std::cout << "|  "  << std::left  << std::setw(3) << i++
                          << "| "  << std::setw(14) << shortAddr(w.getAddress())
                          << "| "  << std::setw(9)  << lock
                          << "| "  << std::right << std::fixed << std::setprecision(2)
                          << std::setw(7) << nodevia.getBalance(w.getAddress())
                          << " NDV |\n";
            }
            std::cout << "+-----+---------------+----------+--------------+\n\n";
        }

        // ── BALANCE ──────────────────────────────────────────────
        else if (cmd == "balance") {
            std::string addr; std::cin >> addr;
            std::cout << "Balance [" << addr << "]: "
                      << std::fixed << std::setprecision(4)
                      << nodevia.getBalance(addr) << " NDV\n";
        }

        // ── MINE ─────────────────────────────────────────────────
        else if (cmd == "mine") {
            std::string addr; std::cin >> addr;
            if (!findWallet(addr))
                std::cout << "[WARN] Address not in local wallets (mining anyway).\n";
            std::cout << "Mining to " << addr << " ...\n";
            nodevia.minePendingTransactions(addr);
        }

        // ── SEND ─────────────────────────────────────────────────
        else if (cmd == "send") {
            std::string from, to; double amt;
            std::cin >> from >> to >> amt;

            Wallet* w = findWallet(from);
            if (!w) { std::cout << "[ERROR] Sender wallet not found.\n"; continue; }

            std::string pass = "";
            if (!w->getPasswordHash().empty()) {
                std::cout << "Password for " << from << ": ";
                std::cin >> pass;
                if (!w->checkPassword(pass)) {
                    std::cout << "[ERROR] Wrong password.\n"; continue;
                }
            }

            Transaction tx(from, to, amt, 1.0);
            tx.publicKey = w->getPublicKey();
            tx.signature = w->sign(tx.signingData());

            if (nodevia.addTransaction(tx))
                std::cout << "Transaction signed and queued: "
                          << amt << " NDV -> " << to << "\n";
            // if rejected, addTransaction already printed the reason
        }

        // ── HISTORY ──────────────────────────────────────────────
        else if (cmd == "history") {
            std::string addr; std::cin >> addr;
            auto txns = nodevia.getAllTransactions();
            int found = 0;
            std::cout << "\nHistory for " << addr << ":\n";
            std::cout << "+--------+----------+----------------------+\n";
            std::cout << "| Type   |   Amount | Counterparty         |\n";
            std::cout << "+--------+----------+----------------------+\n";
            for (auto& tx : txns) {
                if (tx.sender != addr && tx.receiver != addr) continue;
                found++;
                bool sent = (tx.sender == addr);
                std::string other = sent ? tx.receiver : tx.sender;
                std::cout << "| " << std::left  << std::setw(7) << (sent?"[SENT]":"[RECV]")
                          << "| " << std::right << std::fixed << std::setprecision(4)
                          << std::setw(8) << tx.amount
                          << " | " << std::left << std::setw(20) << shortAddr(other) << "|\n";
            }
            std::cout << "+--------+----------+----------------------+\n";
            if (!found) std::cout << "  No transactions found.\n";
            std::cout << "\n";
        }

        // ── EXPORT ───────────────────────────────────────────────
        else if (cmd == "export") {
            std::string addr; std::cin >> addr;
            Wallet* w = findWallet(addr);
            if (!w) { std::cout << "[ERROR] Wallet not found.\n"; continue; }
            std::string pass = "";
            if (!w->getPasswordHash().empty()) {
                std::cout << "Password: "; std::cin >> pass;
                if (!w->checkPassword(pass)) {
                    std::cout << "[ERROR] Wrong password.\n"; continue;
                }
            }
            std::cout << "\nSENSITIVE - Do not share!\n";
            std::cout << "  Address    : " << w->getAddress()        << "\n";
            std::cout << "  Public Key : " << w->getPublicKey()      << "\n";
            std::cout << "  Seed       : " << w->getSeed()           << "\n";
            std::cout << "  Private Key: " << w->getPrivateKey(pass) << "\n\n";
        }

        // ── PASSWD ───────────────────────────────────────────────
        else if (cmd == "passwd") {
            std::string addr; std::cin >> addr;
            Wallet* w = findWallet(addr);
            if (!w) { std::cout << "[ERROR] Wallet not found.\n"; continue; }
            std::string oldPass="", newPass="";
            if (!w->getPasswordHash().empty()) {
                std::cout << "Current password: "; std::cin >> oldPass;
            }
            std::cout << "New password (- to remove): "; std::cin >> newPass;
            if (newPass == "-") newPass = "";
            if (w->setPassword(oldPass, newPass)) {
                std::cout << "Password updated.\n"; saveWallets();
            } else {
                std::cout << "[ERROR] Current password incorrect.\n";
            }
        }

        // ── HELP ─────────────────────────────────────────────────
        else if (cmd == "help") { printHelp(); }

        // ── EXIT ─────────────────────────────────────────────────
        else if (cmd == "exit" || cmd == "quit") {
            saveWallets();
            std::cout << "Goodbye\n";
            break;
        }

        else {
            std::cout << "[ERROR] Unknown command: '" << cmd << "'. Type 'help'.\n";
        }
    }
    return 0;
}