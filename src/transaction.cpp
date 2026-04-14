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
#include "transaction.h"
#include "utils.h"
#include <ctime>
#include <atomic>

// Monotonic counter — ensures txid is unique even for identical field values
static std::atomic<uint64_t> txCounter{0};

Transaction::Transaction(std::string s, std::string r, double a, double f)
    : sender(s), receiver(r), amount(a), fee(f)
{
    uint64_t seq = ++txCounter;
    txid = sha256(sender + receiver
                + std::to_string(amount) + std::to_string(fee)
                + std::to_string((long)time(0))
                + std::to_string(seq));
}

std::string Transaction::signingData() const {
    return sender + receiver + std::to_string(amount) + std::to_string(fee);
}

std::string Transaction::toString() const {
    return txid + signingData() + signature + publicKey;
}