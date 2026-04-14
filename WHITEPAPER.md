**Nodevia: A Simplified Blockchain System**

Akshaj Chiguru

*Nodevia Whitepaper v0.1 --- Educational Blockchain Implementation in
C++*

**Abstract**

> Nodevia (NDV) is a simplified blockchain system implemented in C++17
> for educational purposes. It demonstrates the core principles of
> decentralized ledger technology, including block creation,
> proof-of-work mining, wallet management, and transaction validation.
> The system operates as a single-node implementation with persistent
> file-based storage, a command-line interface, SHA-256 cryptographic
> hashing, and a proof-of-work consensus mechanism. This paper describes
> the system architecture, transaction model, mining incentive
> structure, and wallet design of Nodevia v0.1, along with its known
> limitations and directions for future development.

**1. Introduction**

The concept of a distributed ledger secured through cryptographic
hashing and a proof-of-work consensus mechanism was introduced by
Nakamoto in 2008, giving rise to the broader field of blockchain
technology. While production blockchain networks are inherently complex
distributed systems, the fundamental principles underlying them can be
demonstrated clearly in a self-contained, single-node environment.

Nodevia (NDV) is such an implementation. Written in C++17 and inspired
by the design principles of early blockchain systems such as Bitcoin,
Nodevia serves as an educational tool for understanding hashing, mining,
transaction validation, and wallet management without the additional
complexity of peer-to-peer networking or external frameworks.

This document describes the design and implementation of Nodevia v0.1,
covering its block structure, transaction model, proof-of-work
mechanism, incentive system, and wallet architecture.

**2. System Overview**

Nodevia operates as a single-node blockchain with persistent storage and
a command-line interface (CLI). The system comprises five primary
components:

> • Blockchain ledger persisted to blockchain.dat
>
> • Wallet registry persisted to wallets.dat
>
> • Command-line interface for all user interactions
>
> • Custom SHA-256 hashing implementation
>
> • Proof-of-work consensus mechanism

Each block in the chain contains a set of validated transactions linked
to the preceding block through its cryptographic hash. This linkage
ensures that modification of any historical block invalidates all
subsequent blocks, providing a tamper-evident record without requiring a
trusted central authority.

**3. Transactions**

A transaction in Nodevia represents the transfer of NDV tokens between
two wallet addresses. Each transaction record contains the following
fields:

> • Sender address
>
> • Receiver address
>
> • Transfer amount
>
> • Transaction fee
>
> • Transaction identifier (txid)

***A.*** Validation Rules

Before a transaction is accepted into the pending pool, it is subjected
to the following validation checks:

> 1\. The transfer amount must be strictly greater than zero.
>
> 2\. The sender must hold a balance sufficient to cover both the
> transfer amount and the associated fee.
>
> 3\. The transaction must reference a valid, existing sender address.

***B.*** Mempool

Validated transactions are held in a pending mempool until a mining
operation includes them in a new block. Each transaction\'s fee is added
to the miner\'s reward upon successful block creation, incentivizing the
timely inclusion of pending transactions.

**4. Blockchain Structure**

Each block in the Nodevia chain contains the following data fields:

> • Block index
>
> • UNIX timestamp of creation
>
> • Nonce value determined by proof-of-work
>
> • Hash of the preceding block
>
> • SHA-256 hash of the current block
>
> • Ordered list of included transactions

Blocks are linked by embedding the hash of the previous block within the
current block\'s data prior to hashing. Any alteration to a historical
block\'s content yields a different hash, breaking the chain at that
point and making tampering immediately detectable through hash
verification.

***A.*** Genesis Block

The genesis block forms the immutable foundation of the chain and
carries no prior block reference. It contains the following inscribed
statement, serving as a verifiable timestamp anchor:

*"The Times 2026 --- Nodevia begins"*

This statement verifies that the genesis block could not have been
created before the referenced date, establishing a lower bound on the
chain\'s origin.

**5. Proof of Work**

Nodevia employs a simplified proof-of-work mechanism to regulate block
creation and impose a computational cost on chain extension. The
algorithm requires a node to find a nonce value such that the SHA-256
hash of the block data satisfies a target condition defined by a
required number of leading zero characters.

***A.*** Algorithm

> 1\. Initialize nonce to zero.
>
> 2\. Compute the SHA-256 hash of the concatenated block data and
> current nonce.
>
> 3\. Evaluate whether the hash satisfies the difficulty target (i.e.,
> begins with the required count of leading zeros).
>
> 4\. If unsatisfied, increment the nonce.
>
> 5\. Repeat steps 2--4 until a valid hash is found.

***B.*** Difficulty

The difficulty parameter governs the expected computational effort
required to produce a valid block. A higher difficulty demands more
leading zeros in the target hash, increasing the average number of
iterations before a valid nonce is found. This mechanism ensures genuine
computational expenditure is required for chain extension.

**6. Incentive System**

To compensate for the computational effort required by proof-of-work,
Nodevia provides a reward to the node that successfully mines each new
block. The reward has two components:

> • A fixed base reward of 10 NDV per block mined.
>
> • The aggregate sum of all transaction fees within the mined block.

The base reward ensures that mining remains viable even when transaction
volume is low. The fee component creates an incentive for miners to
prioritize pending transactions, maximizing throughput. As transaction
volume grows, fee revenue constitutes a progressively larger proportion
of the miner\'s total compensation, providing a sustainable incentive
model independent of the base reward.

**7. Wallet System**

The Nodevia wallet system provides address generation and basic key
management through the CLI. Wallet creation follows a structured
derivation procedure:

> 1\. A random seed is generated as the entropy source.
>
> 2\. A private key is derived deterministically from the seed.
>
> 3\. A wallet address is produced by hashing the seed.
>
> 4\. An optional password is associated using a hash-based locking
> mechanism.

***A.*** Supported Operations

> • Wallet creation via the CLI
>
> • Export of seed phrase and private key, subject to password
> authentication
>
> • Password-based locking and unlocking of wallet access

***B.*** Security Note

Nodevia v0.1 does not implement full cryptographic digital signatures.
The security model is deliberately simplified and is appropriate only
for educational use. Wallet data is persisted to wallets.dat and loaded
at system startup.

**8. Implementation**

Nodevia is implemented in C++17 using only the standard template library
(STL) and a custom SHA-256 implementation. No external blockchain
frameworks, cryptographic libraries, or networking dependencies are
required. All persistence is achieved through binary file I/O to
blockchain.dat and wallets.dat, and all user interaction is conducted
through the command-line interface.

The absence of external dependencies is a deliberate design choice to
ensure that the entire system logic is transparent and self-contained,
making it accessible for study and modification by developers at all
levels of experience.

**9. Limitations**

Nodevia v0.1 is an educational prototype. The following limitations
preclude its use in any production context:

> • No peer-to-peer networking layer; all operations are confined to a
> single machine.
>
> • No multi-node consensus protocol; there is no mechanism to reconcile
> divergent chain states.
>
> • No smart contract execution environment.
>
> • No full cryptographic digital signature scheme; the security model
> is simplified.
>
> • Single-machine execution only; the system cannot participate in a
> distributed network.

These constraints are inherent to the scope of the v0.1 release and are
acknowledged as the primary directions for future engineering effort.

**10. Future Work**

The following improvements are planned for subsequent versions of
Nodevia:

> • Peer-to-peer network layer enabling communication between multiple
> independent nodes.
>
> • Blockchain synchronization protocol for maintaining consensus across
> a distributed network.
>
> • Full cryptographic digital signature scheme for secure transaction
> authentication.
>
> • Enhanced wallet security, including hardware-grade entropy sources
> and robust key derivation.
>
> • Performance optimizations to the mining engine and file persistence
> subsystems.
>
> • Optional graphical user interface (GUI) for non-technical users.

**11. Conclusion**

Nodevia demonstrates the fundamental mechanics of blockchain technology
within a contained, single-node environment. By implementing block
creation, proof-of-work mining, transaction validation, and wallet
management using only standard C++ and a custom SHA-256 library, Nodevia
exposes the essential logic of distributed ledger systems in a form that
is accessible, inspectable, and fully self-contained.

The system is not intended for production use. Its value lies in
providing an unobscured view of the principles that underpin modern
blockchain networks, serving as a foundation for deeper study and
experimentation.

**Genesis Statement**

*"The Times 2026 --- Nodevia begins"*

*Nodevia (NDV) Whitepaper v0.1 · Akshaj Chiguru · 2026 · For educational
use only.*
