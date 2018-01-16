//
//  blockChain.cpp
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#include <stack>
#include "blockChain.h"
#include "crypto.h"
#include "block.h"
#include "factory.h"
#include "talk.h"
#include "dialog.h"

string blockChain::CalculateHash(const int index, const string& preHash, const time_t& timeStamp, const string& data)
{
    string s(to_string(index) + data + to_string(timeStamp) + preHash);
    s = crypto::SHA256(s.c_str(), (int)s.size());
    return s;
}

blockChain::blockChain(bool bGenerateGenesis)
{
    dialog* dialog = factory::GetDialog();
    dialog->appendLog("Initializing blockchain...");

    head = NULL;
    tail = NULL;
    len = 0;

    if(bGenerateGenesis)
        addBlock(getGenesisBlock());

    dialog->appendLog("Blockchain initialized.");
}

block* blockChain::getGenesisBlock()
{
    return new block(0, "816534932c2b7154836da6afc367695e6337db8a921823784c14378abed4f7d7", 1513598789, dialog::GetCommand(dialog::DIALOG_COMMAND_TYPE_UPDATEALL, 0, ""), "cbe8af26e86a0a0e88f91c30b4b12239f7ac2b1bf006b647d05711234f82ef35");
}

block* blockChain::generateNextBlock(const string& data)
{
    if(data.empty())
    {
        factory::GetDialog()->appendLog(QString("Invalid block name : [").append(data.c_str()).append("], cannot generate new block."));
        return NULL;
    }

    time_t timeStamp;
    time(&timeStamp);
    return new block(tail->getIndex() + 1, tail->getHash(), timeStamp, data, CalculateHash(tail->getIndex() + 1, tail->getHash(), timeStamp, data));
}

void blockChain::addBlock(block *newBlock)
{
    if(!newBlock)
    {
        factory::GetDialog()->appendLog("No new block is added.");
        return;
    }

    head = !head ? newBlock : head;
    tail = newBlock;
    blockHash[newBlock->getHash()] = newBlock;
    ++len;
}

bool blockChain::IsValidBlock(const int index, const string& preHash, const time_t& timeStamp, const string& data, const string& hash, block* preBlock)
{
    dialog* dialog = factory::GetDialog();

    if(preBlock->getIndex() + 1 != index)
    {
        dialog->appendLog("Invalid index");
        return false;
    }

    if(preBlock->getHash() != preHash)
    {
        dialog->appendLog("Invalid previoushash");
        return false;
    }

    string newHash = CalculateHash(index, preHash, timeStamp, data);

    if(newHash != hash)
    {
        dialog->appendLog(QString("Invalid hash: ").append(newHash.c_str()).append(" ").append(hash.c_str()));
        return false;
    }

    return true;
}

bool blockChain::IsValidChain(blockChain * const chain)
{
    block *cur = chain->getLatestBlock();

    while(cur)
    {
        block *pre = chain->getBlock(cur->getPreHash());

        if(!pre)
            return true;

        if(!IsValidBlock(cur->getIndex(), cur->getPreHash(), cur->getTimeStamp(), cur->getData(), cur->getHash(), pre))
            return false;

        cur = pre;
    }

    return true;
}

void blockChain::replaceChain(blockChain * const newChain)
{
    dialog* dialog = factory::GetDialog();

    if(!IsValidChain(newChain) || newChain->length() <= len)
    {
        dialog->appendLog("New blockchain is invalid or shorter than current one, not replaced.");
        return;
    }

    dialog->appendLog("New blockchain is valid. Replacing current blockchain with received blockchain.");

    removeAll();
    stack<block*> s;

    block* cur = newChain->getLatestBlock();

    while(cur)
    {
        s.push(cur);
        block *pre = newChain->getBlock(cur->getPreHash());
        blockHash[cur->getHash()] = new block(cur);
        tail = !tail ? blockHash[cur->getHash()] : tail;
        head = blockHash[cur->getHash()];
        cur = pre;
    }

    while(!s.empty())
    {
        factory::GetDialog()->updateRemoteDoc(s.top()->getData().c_str());
        s.pop();
    }

    len = newChain->length();
}

string blockChain::getChainInfo()
{
    string res;
    char wrap = ',';

    block* cur = getLatestBlock();

    while(cur)
    {
        if(!res.empty())
            res += wrap;

        res += cur->getBlockInfo();
        cur = getBlock(cur->getPreHash());
    }

    return res;
}

blockChain* blockChain::GenerateChain(const string& chainInfo)
{
    string info = chainInfo;
    size_t found = info.find(",");
    int index;
    string preHash, data, hash;
    time_t timeStamp;
    stack<block*> s;

    factory::GetDialog()->appendLog(QString("generateChain ").append(chainInfo.c_str()));

    while(found != string::npos)
    {
        block::TransferInfo(info.substr(0, found), index, preHash, timeStamp, data, hash);
        s.push(new block(index, preHash, timeStamp, data, hash));
        info = info.substr(found + 1);
        found = info.find(",");
    }

    block::TransferInfo(info, index, preHash, timeStamp, data, hash);
    s.push(new block(index, preHash, timeStamp, data, hash));
    
    blockChain* newChain = new blockChain();

    while(!s.empty())
    {
        newChain->addBlock(s.top());
        s.pop();
    }

    return newChain;
}

blockChain::~blockChain()
{
    removeAll();
}

void blockChain::removeAll()
{
    block *cur = getLatestBlock();
    
    while(cur)
    {
        block *pre = blockHash[cur->getPreHash()];
        delete cur;
        cur = pre;
    }
    
    blockHash.clear();
    len = 0;
    tail = NULL;
}

