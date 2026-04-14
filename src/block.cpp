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
#include "block.h"
#include "utils.h"
#include <ctime>

Block::Block(int idx, std::vector<Transaction> txs, std::string prev) {
    index        = idx;
    transactions = txs;
    prevHash     = prev;
    timestamp    = time(0);
    nonce        = 0;
    hash         = calculateHash();
}

// Restore constructor: all fields set directly, hash NOT recalculated
Block::Block(int idx, long ts, int nc,
             std::string prev, std::string h,
             std::vector<Transaction> txs) {
    index        = idx;
    timestamp    = ts;
    nonce        = nc;
    prevHash     = prev;
    hash         = h;
    transactions = txs;
}

std::string Block::calculateHash() {
    std::string txData;
    for (const auto& tx : transactions) txData += tx.toString();
    return sha256(
        std::to_string(index)     +
        txData                    +
        prevHash                  +
        std::to_string(timestamp) +
        std::to_string(nonce)
    );
}