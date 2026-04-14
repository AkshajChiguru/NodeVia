# Nodevia (NDV) v0.1

Nodevia is a simplified blockchain system written in C++17 for educational purposes.

It demonstrates:
- Blockchain structure
- Proof of Work mining
- Wallet system
- Transactions
- Persistent storage

---

## Creator
Akshaj Chiguru

---

## Features
- SHA-256 blockchain
- Mining system (Proof of Work)
- Wallet system with password protection
- Transaction system with fees
- File-based persistence

---

## CLI Commands
create, list, send, mine, balance, export, passwd, help, exit

---

## Build
g++ -std=c++17 src/*.cpp -Iinclude -o nodevia

---

## Run
./nodevia

---

## Limitations
- No networking
- No multi-node system
- No smart contracts