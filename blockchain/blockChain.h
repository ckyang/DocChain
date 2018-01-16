//
//  blockChain.h
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#ifndef blockChain_h
#define blockChain_h

#include <string>
#include <unordered_map>

using namespace std;

class block;

class blockChain
{
public:
    blockChain(bool bGenerateGenesis = false);
    ~blockChain();

    static bool IsValidChain(blockChain * const chain);
    static bool IsValidBlock(const int index, const string& preHash, const time_t& timeStamp, const string& data, const string& hash, block *preBlock);
    static blockChain* GenerateChain(const string& chainInfo);

    block* generateNextBlock(const string& data);
    void addBlock(block *newBlock);
    unsigned long long length(){return len;}
    block *getLatestBlock(){return tail;}
    block *getBlock(const string& hash){return blockHash[hash];}
    void replaceChain(blockChain * const chain);
    string getChainInfo();

private:
    static string CalculateHash(const int index, const string& preHash, const time_t& timeStamp, const string& data);

    block *getGenesisBlock();
    void removeAll();

    block *head;
    block *tail;
    unordered_map<string, block*> blockHash;
    unsigned long long len;
};

#endif /* blockChain_h */
