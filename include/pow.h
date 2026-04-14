#ifndef POW_H
#define POW_H
#include "block.h"
class ProofOfWork {
public:
    static void mine(Block& block, int difficulty);
};
#endif