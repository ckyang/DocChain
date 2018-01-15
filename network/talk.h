//
//  talk.h
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#ifndef talk_h
#define talk_h

#include <string>
#include <unordered_map>

#define REMOTE_COMMAND_NEW "NEW"
#define REMOTE_COMMAND_GET_LAST "GET_LAST"
#define REMOTE_COMMAND_GET_ALL "GET_ALL"
#define REMOTE_COMMAND_GET_PUBKEY "GET_PUBKEY"
#define REMOTE_COMMAND_REPLY_LAST "REPLY_LAST"
#define REMOTE_COMMAND_REPLY_ALL "REPLY_ALL"
#define REMOTE_COMMAND_REPLY_PUBKEY "REPLY_PUBKEY"

using namespace std;

class blockChain;
class block;

class talk
{
public:
    talk(){};

    static void Broadcast(const string& message);
    static void Broadcast(const char* str, int len);
    static void BroadcastPublicKey();
    static void ItoHex(int number, char hex[]);
    static void HextoI(int& number, const char hex[]);

    void connect();

private:
    static void Response(int sock_fd, short event, void *arg);
    static bool StartWith(const char* str, int len, const char* start_str);

    //Message Format: [BLOCKCHAIN_HEADER][ADDRESS][MessageLen][Message][Signature]
    //BLOCKCHAIN_HEADER(strlen(BLOCKCHAIN_HEADER)): "__ThisIsBlockChainPacketByClarkYang__", to avoid conflicting to other network packets
    //Address(40): Identify the user of this protocol
    //MessageLen(4): (0000 ~ FFFF) Indicate message length
    //Message(MessageLen): Transferred message
    //Signature(-) : Message signature

    static void SendMsg(const int sock_fd, const struct sockaddr_in* sock_in, const string& msg);
    static void SendMsg(const int sock_fd, const struct sockaddr_in* sock_in, const char* msg, const int len);
    static bool RcvMsg(const int sock_fd, struct sockaddr_in* client_addr, char* msg, int& len);
    static void GetReplyPublicKey(char* writeBuf, int& len);

    static unordered_map<string, pair<unsigned char*, int>> pubKeyHash;
};

#endif /* talk_h */
