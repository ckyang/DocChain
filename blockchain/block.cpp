//
//  block.cpp
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#include "block.h"

block::block(block * const b)
{
    m_index = b->getIndex();
    m_preHash = b->getPreHash();
    m_timeStamp = b->getTimeStamp();
    m_data = b->getData();
    m_hash = b->getHash();
}

block::block(const int index, const string& preHash, const time_t& timeStamp, const string& data, const string& hash)
{
    m_index = index;
    m_preHash = preHash;
    m_timeStamp = timeStamp;
    m_data = data;
    m_hash = hash;
}

void block::TransferInfo(const string& totalInfo, int& index, string& preHash, time_t& timeStamp, string& data, string& hash)
{
    string info = totalInfo;
    size_t found = info.find(" ");
    index = stoi(info.substr(0, found));

    info = info.substr(found + 1);
    found = info.find(" ");
    preHash = info.substr(0, found);

    info = info.substr(found + 1);
    found = info.find(" ");
    timeStamp = stoll(info.substr(0, found));

    info = info.substr(found + 1);
    found = info.find(" ");

    if(info[found + 1] == ' ')
        ++found;

    data = info.substr(0, found);
    hash = info.substr(found + 1);
}
