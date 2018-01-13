//
//  crypto.h
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#ifndef crypto_h
#define crypto_h

#include <string>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>

using namespace std;

#define MAX_SIGNATURE_LEN 256
#define MAX_PUBLICKEY_LEN 256

class crypto
{
public:
    crypto();
    ~crypto();

    static string SHA256(const char* data, const unsigned int len);
    static string RIPEMD160(const char* data, const unsigned int len);

    void getPublicKey(unsigned char *pubKey, unsigned int& pubKeyLen);
    const string& getAddress();
    void sign(const char* msg, const int len, unsigned char *signature, unsigned int& signatureLen);
    bool verify(const char* msg, const int len, const unsigned char *signature, const unsigned int signatureLen, const unsigned char *pubKey, const unsigned int pubKeyLen);

private:
    void generateKeyPair();

    EC_KEY* m_pECCKeyPair;
    string m_address;
};

#endif /* crypto_h */
