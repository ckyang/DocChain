//
//  talk.cpp
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#include "talk.h"
#include "unistd.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "event.h"
#include "factory.h"
#include "blockChain.h"
#include "block.h"
#include "dialog.h"
#include "crypto.h"

#define SENDER_PORT 7000
#define RECEIVER_PORT 7000
#define MAX_BUF_SIZE 65535
#define BLOCKCHAIN_HEADER "__ThisIsBlockChainPacketByClarkYang__"

using namespace std;

unordered_map<string, pair<unsigned char*, int>> talk::pubKeyHash;

void talk::Response(int sock_fd, short event, void *arg)
{
    (void)event;
    (void)arg;

    struct sockaddr_in client_addr;
    dialog* dialog = factory::GetDialog();
    static char readBuf[MAX_BUF_SIZE];
    memset(readBuf, '\0', MAX_BUF_SIZE);
    int len = 0;

    if(!RcvMsg(sock_fd, &client_addr, readBuf, len))
        return;

    client_addr.sin_port = htons(SENDER_PORT);

    if(len < 7)
    {
        string writeBuf = "Invalid command!";
        SendMsg(sock_fd, &client_addr, writeBuf);
        return;
    }

    blockChain* blockChainObject = factory::GetBlockChain();

    //ASK_VERIFY blockInfo
    if(StartWith(readBuf, len, REMOTE_COMMAND_ASK_VERIFY))
    {
        int index;
        string preHash, data, hash;
        time_t timeStamp;
        string tmp(readBuf + strlen(REMOTE_COMMAND_ASK_VERIFY) + 1, len - strlen(REMOTE_COMMAND_ASK_VERIFY) - 1);
        block::TransferInfo(tmp, index, preHash, timeStamp, data, hash);

        //If this block is already the top of blockchain, do nothing
        if(blockChainObject->getLatestBlock()->getIndex() == index && blockChainObject->getLatestBlock()->getPreHash() == preHash && blockChainObject->getLatestBlock()->getTimeStamp() == timeStamp && blockChainObject->getLatestBlock()->getData() == data && blockChainObject->getLatestBlock()->getHash() == hash)
        {
            dialog->appendLog("This block already in the top of blockchain, do nothing.");
            return;
        }

        //If this block (coming from remote) is valid, return validate +1.
        //If it's invalid, request the whole blockchain.
        if(blockChain::IsValidBlock(index, preHash, timeStamp, data, hash, blockChainObject->getLatestBlock()))
        {
            string writeBuf = string(REMOTE_COMMAND_CONFIRM_VERIFY) + " " + hash;
            SendMsg(sock_fd, &client_addr, writeBuf);
            return;
        }

        string writeBuf = REMOTE_COMMAND_GET_ALL;
        SendMsg(sock_fd, &client_addr, writeBuf);
        return;
    }

    //COMFIRM_VERIFY hash
    if(StartWith(readBuf, len, REMOTE_COMMAND_CONFIRM_VERIFY))
    {
        string hash(readBuf + strlen(REMOTE_COMMAND_CONFIRM_VERIFY) + 1, len - strlen(REMOTE_COMMAND_CONFIRM_VERIFY) - 1);
        factory::GetDialog()->accumulateValidation(QString(hash.c_str()));
        return;
    }

    //NEW blockInfo
    if(StartWith(readBuf, len, REMOTE_COMMAND_NEW))
    {
        int index;
        string preHash, data, hash;
        time_t timeStamp;
        string tmp(readBuf + strlen(REMOTE_COMMAND_NEW) + 1, len - strlen(REMOTE_COMMAND_NEW) - 1);
        block::TransferInfo(tmp, index, preHash, timeStamp, data, hash);

        //If this block (coming from remote) is valid, add this block.
        //If it's invalid, request the whole blockchain.
        if(blockChain::IsValidBlock(index, preHash, timeStamp, data, hash, blockChainObject->getLatestBlock()))
        {
            factory::GetBlockChain()->addBlock(new block(index, preHash, timeStamp, data, hash));
            factory::GetDialog()->updateBlockChainList();
        }
        else
        {
            string writeBuf = REMOTE_COMMAND_GET_ALL;
            SendMsg(sock_fd, &client_addr, writeBuf);
        }

        return;
    }

    //GET_LAST
    if(StartWith(readBuf, len, REMOTE_COMMAND_GET_LAST))
    {
        string writeBuf = string(REMOTE_COMMAND_REPLY_LAST) + " " + blockChainObject->getLatestBlock()->getBlockInfo();

        SendMsg(sock_fd, &client_addr, writeBuf);
        return;
    }

    //GET_ALL
    if(StartWith(readBuf, len, REMOTE_COMMAND_GET_ALL))
    {
        string writeBuf = string(REMOTE_COMMAND_REPLY_ALL) + " " + blockChainObject->getChainInfo();
        SendMsg(sock_fd, &client_addr, writeBuf);
        return;
    }

    //REPLY_LAST ...
    if(StartWith(readBuf, len, REMOTE_COMMAND_REPLY_LAST))
    {
        //TBD

        return;
    }

    //REPLY_ALL
    if(StartWith(readBuf, len, REMOTE_COMMAND_REPLY_ALL))
    {
        string tmp(readBuf + strlen(REMOTE_COMMAND_REPLY_ALL) + 1);
        tmp[len - strlen(REMOTE_COMMAND_REPLY_ALL)] = '\0';
        blockChain* newChain = blockChain::GenerateChain(tmp);

        //If other node's blockchain is longer than current, use this one
        if(newChain->length() > blockChainObject->length())
        {
            blockChainObject->replaceChain(newChain);
            factory::GetDialog()->updateBlockChainList();
        }

        delete newChain;
        return;
    }

    //GET_PUBKEY
    if(StartWith(readBuf, len, REMOTE_COMMAND_GET_PUBKEY))
    {
        static char writeBuf[MAX_BUF_SIZE];
        int len = 0;
        GetReplyPublicKey(writeBuf, len);
        SendMsg(sock_fd, &client_addr, writeBuf, len);
        return;
    }

    //REMOTE_COMMAND_REPLY_PUBKEY address publicKey
    if(StartWith(readBuf, len, REMOTE_COMMAND_REPLY_PUBKEY))
    {
        crypto* crypto = factory::GetCrypto();
        char *senderAddress = (char*)malloc((crypto->getAddress().size() + 1) * sizeof(char));
        memcpy(senderAddress, readBuf + strlen(REMOTE_COMMAND_REPLY_PUBKEY) + 1, crypto->getAddress().size());
        senderAddress[crypto->getAddress().size()] = '\0';
        int pubKeyLen = len - strlen(REMOTE_COMMAND_REPLY_PUBKEY) - 1 - (int)crypto->getAddress().size();
        unsigned char* pubKey = (unsigned char*)malloc(sizeof(unsigned char) * pubKeyLen);
        memcpy(pubKey, readBuf + strlen(REMOTE_COMMAND_REPLY_PUBKEY) + 1 + crypto->getAddress().size(), pubKeyLen);
        pubKeyHash[senderAddress] = make_pair(pubKey, pubKeyLen);
        free(senderAddress);
        return;
    }
}

void talk::connect()
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(1);
    }

    int yes = 1;

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
        perror("setsockopt()");
        exit(1);
    }

    struct sockaddr_in sock_in;
    bzero(&sock_in, sizeof(sock_in));
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(RECEIVER_PORT);
    sock_in.sin_addr.s_addr = INADDR_ANY;
    if (::bind(sock_fd, (struct sockaddr *) &sock_in, sizeof(sock_in)) < 0) {
        perror("bind");
        exit(1);
    }

    event_init();

    struct event ev;
    event_set(&ev, sock_fd, EV_READ | EV_PERSIST, Response, &ev);

    event_add(&ev, NULL);

    event_dispatch();
    shutdown(sock_fd, 2);
    close(sock_fd);
}

void talk::Broadcast(const string& message)
{
    Broadcast(message.c_str(), (int)message.size());
}

void talk::Broadcast(const char* str, int len)
{
    int sock_fd;
    struct sockaddr_in sock_in;
    int yes = 1;

    memset(&sock_in, 0, sizeof(struct sockaddr_in));

    sock_fd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_in.sin_port = htons(0);
    sock_in.sin_family = PF_INET;

    if (::bind(sock_fd, (struct sockaddr *) &sock_in, sizeof(struct sockaddr_in)) < 0) {
        perror("bind");
        exit(1);
    }

    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int));

    sock_in.sin_addr.s_addr = htonl(-1); /* send message to 255.255.255.255 */
    sock_in.sin_port = htons(SENDER_PORT);
    sock_in.sin_family = PF_INET;

    SendMsg(sock_fd, &sock_in, str, len);

    factory::GetDialog()->appendLog("Message is broadcasted.");
//    factory::GetDialog()->appendLog(message.c_str());

    shutdown(sock_fd, 2);
    close(sock_fd);
}

void talk::BroadcastPublicKey()
{
    static char writeBuf[MAX_BUF_SIZE];
    int len = 0;
    GetReplyPublicKey(writeBuf, len);
    Broadcast(writeBuf, len);
}

bool talk::StartWith(const char* str, int len, const char* start_str)
{
    if(len < (int)strlen(start_str))
        return false;

    for(int i = 0; i < (int)strlen(start_str); ++i)
        if(str[i] != start_str[i])
            return false;

    return true;
}

void talk::SendMsg(const int sock_fd, const struct sockaddr_in* sock_in, const string& msg)
{
    SendMsg(sock_fd, sock_in, msg.c_str(), (int)msg.size());
}

void talk::SendMsg(const int sock_fd, const struct sockaddr_in* sock_in, const char* msg, const int len)
{
    if(0 == len)
        return;

    unsigned char signature[MAX_SIGNATURE_LEN] = {'\0'};
    unsigned int signatureLen = 0;
    crypto* crypto = factory::GetCrypto();
    crypto->sign(msg, len, signature, signatureLen);

    static char message[MAX_BUF_SIZE], lenHex[4] = {'0'};

    ItoHex(len, lenHex);
    memset(message, '\0', MAX_BUF_SIZE);
    memcpy(message, BLOCKCHAIN_HEADER, strlen(BLOCKCHAIN_HEADER));
    memcpy(message + strlen(BLOCKCHAIN_HEADER), crypto->getAddress().c_str(), crypto->getAddress().size());
    memcpy(message + strlen(BLOCKCHAIN_HEADER) + crypto->getAddress().size(), lenHex, 4);
    memcpy(message + strlen(BLOCKCHAIN_HEADER) + crypto->getAddress().size() + 4, msg, len);
    memcpy(message + strlen(BLOCKCHAIN_HEADER) + crypto->getAddress().size() + 4 + len, signature, signatureLen);

    sendto(sock_fd, message, strlen(BLOCKCHAIN_HEADER) + crypto->getAddress().size() + 4 + len + signatureLen + 1, 0, (struct sockaddr *)sock_in, sizeof(struct sockaddr_in));
}

bool talk::RcvMsg(const int sock_fd, struct sockaddr_in* client_addr, char* msg, int& len)
{
    static char rbuf[MAX_BUF_SIZE];
    int size = sizeof(struct sockaddr), receiveLen;
    memset(rbuf, '\0', MAX_BUF_SIZE);
    dialog* dialog = factory::GetDialog();

    if((receiveLen = (int)recvfrom(sock_fd, rbuf, (size_t)sizeof(rbuf), 0, (struct sockaddr *)client_addr, (socklen_t *)&size) - 1) < 0)
        return false;

    if(!StartWith(rbuf, receiveLen, BLOCKCHAIN_HEADER))
        return false;

    crypto* crypto = factory::GetCrypto();

    char *senderAddress = (char*)malloc((crypto->getAddress().size() + 1) * sizeof(char));
    memcpy(senderAddress, rbuf + strlen(BLOCKCHAIN_HEADER), crypto->getAddress().size());
    senderAddress[crypto->getAddress().size()] = '\0';

    if(crypto->getAddress() == senderAddress)
    {
        dialog->appendLog("Received packet from myself, ignore it.");
        free(senderAddress);
        return false;
    }

    HextoI(len, rbuf + strlen(BLOCKCHAIN_HEADER) + crypto->getAddress().size());
    memcpy(msg, rbuf + strlen(BLOCKCHAIN_HEADER) + crypto->getAddress().size() + 4, len);

    dialog->appendLog(QString("Received packet from [").append(senderAddress).append("]"));
    dialog->appendLog(QString("Received: ").append(msg));

    //Don't validate signature with REMOTE_COMMAND_REPLY_PUBKEY
    if(StartWith(msg, len, REMOTE_COMMAND_REPLY_PUBKEY))
        return true;

    unsigned char signature[MAX_SIGNATURE_LEN] = {'\0'};
    unsigned int signatureLen = receiveLen - strlen(BLOCKCHAIN_HEADER) - (int)crypto->getAddress().size() - 4 - len - ('\n' == rbuf[receiveLen - 1] ? 1 : 0);
    memcpy(signature, rbuf + strlen(BLOCKCHAIN_HEADER) + crypto->getAddress().size() + 4 + len, signatureLen);

    if(pubKeyHash.find(senderAddress) == pubKeyHash.end())
    {
        dialog->appendLog("No public key, cannot validate!");
        free(senderAddress);
        return false;
    }

    if(!factory::GetCrypto()->verify(msg, len, signature, signatureLen, pubKeyHash[senderAddress].first, pubKeyHash[senderAddress].second))
    {
        dialog->appendLog("Verify NG!");
        free(senderAddress);
        return false;
    }

    dialog->appendLog("Verify OK!");
    free(senderAddress);
    return true;
}

void talk::ItoHex(int number, char hex[])
{
    for(int i = 0; i < 4; ++i)
        hex[i] = '0';

    int idx = 3;

    while(number > 0)
    {
        hex[idx--] = (number % 16 < 10) ? (number % 16 + '0') : (number % 16 - 10 + 'A');
        number /= 16;
    }
}

void talk::HextoI(int& number, const char hex[])
{
    number = 0;

    for(int i = 0; i < 4; ++i)
        number = number * 16 + ((hex[i] <= '9' && hex[i] >= '0') ? (hex[i] - '0') : (hex[i] - 'A' + 10));
}

void talk::GetReplyPublicKey(char* writeBuf, int& len)
{
    unsigned char pubKey[MAX_PUBLICKEY_LEN] = {'\0'};
    unsigned int pubKeyLen = 0;
    crypto* crypto = factory::GetCrypto();
    crypto->getPublicKey(pubKey, pubKeyLen);
    memcpy(writeBuf, REMOTE_COMMAND_REPLY_PUBKEY, strlen(REMOTE_COMMAND_REPLY_PUBKEY));
    writeBuf[strlen(REMOTE_COMMAND_REPLY_PUBKEY)] = ' ';
    memcpy(writeBuf + strlen(REMOTE_COMMAND_REPLY_PUBKEY) + 1, crypto->getAddress().c_str(), crypto->getAddress().size());
    memcpy(writeBuf + strlen(REMOTE_COMMAND_REPLY_PUBKEY) + 1 + crypto->getAddress().size(), pubKey, pubKeyLen);
    len = strlen(REMOTE_COMMAND_REPLY_PUBKEY) + 1 + (int)crypto->getAddress().size() + pubKeyLen;
}

