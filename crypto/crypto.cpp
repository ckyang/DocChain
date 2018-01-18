//
//  crypto.cpp
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ripemd.h>
#include "crypto.h"
#include "factory.h"
#include "dialog.h"

#define ECCTYPE "secp521r1"

crypto::crypto()
{
    generateKeyPair();
    m_address.clear();
}

crypto::~crypto()
{
    EC_KEY_free(m_pECCKeyPair);
}

string crypto::SHA256(const char* data, const unsigned int len)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, len);
    SHA256_Final(hash, &sha256);
    char outputBuffer[SHA256_DIGEST_LENGTH * 2 + 1];

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);

    outputBuffer[SHA256_DIGEST_LENGTH * 2] = 0;
    return string(outputBuffer);
}

string crypto::RIPEMD160(const char* data, const unsigned int len)
{
    unsigned char digest[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160_CTX ctx;
    RIPEMD160_Init(&ctx);
    RIPEMD160_Update(&ctx, data, len);
    RIPEMD160_Final(digest, &ctx);
    char outputBuffer[RIPEMD160_DIGEST_LENGTH * 2 + 1];

    for (int i = 0; i < RIPEMD160_DIGEST_LENGTH; i++)
        sprintf(&outputBuffer[i * 2], "%02x", (unsigned int)digest[i]);

    outputBuffer[RIPEMD160_DIGEST_LENGTH * 2] = 0;
    return string(outputBuffer);
}

void crypto::generateKeyPair()
{
    dialog* dialog = factory::GetDialog();
    dialog->appendLog("Generating public/private key pairs...");

    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    BIO* pbio_key_file  = BIO_new(BIO_s_file());
    pbio_key_file = BIO_new_fp(stdout, BIO_NOCLOSE);
    m_pECCKeyPair = EC_KEY_new_by_curve_name(OBJ_txt2nid(ECCTYPE));
    EC_KEY_set_asn1_flag(m_pECCKeyPair, OPENSSL_EC_NAMED_CURVE);

    if (! (EC_KEY_generate_key(m_pECCKeyPair)))
        BIO_printf(pbio_key_file, "Failed to generate the ECC key.");

    EVP_PKEY* pkey = EVP_PKEY_new();

    if (!EVP_PKEY_assign_EC_KEY(pkey, m_pECCKeyPair))
        BIO_printf(pbio_key_file, "Failed to assign ECC key to EVP_PKEY structure.");

    m_pECCKeyPair = EVP_PKEY_get1_EC_KEY(pkey);

    EVP_PKEY_free(pkey);
    BIO_free_all(pbio_key_file);
    dialog->appendLog("Public/private key pairs generated.");
}

void crypto::getPublicKey(unsigned char *pubKey, unsigned int& pubKeyLen)
{
    pubKeyLen = i2o_ECPublicKey(m_pECCKeyPair, NULL);
    unsigned char *endBuf = pubKey;
    i2o_ECPublicKey(m_pECCKeyPair, &endBuf);
}

const string& crypto::getAddress()
{
    if(m_address.empty())
    {
        unsigned char pubKey[MAX_PUBLICKEY_LEN] = {'\0'};
        unsigned int pubKeyLen = 0;
        getPublicKey(pubKey, pubKeyLen);
        string tmp = SHA256((char*)pubKey, pubKeyLen);
        m_address = RIPEMD160(tmp.c_str(), (int)tmp.size());
        factory::GetDialog()->updateLocalAddress(m_address.c_str());
    }

    return m_address;
}

void crypto::sign(const char* msg, const int len, unsigned char *signature, unsigned int& signatureLen)
{
    string digest = SHA256(msg, len);
    signatureLen = ECDSA_size(m_pECCKeyPair);
    ECDSA_sign(0, (unsigned char*)digest.c_str(), (int)digest.size(), signature, &signatureLen, m_pECCKeyPair);
}

bool crypto::verify(const char* msg, const int len, const unsigned char *signature, const unsigned int signatureLen, const unsigned char *pubKey, const unsigned int pubKeyLen)
{
    if(!signature || 0 == signatureLen || !pubKey || 0 == pubKeyLen)
    {
        factory::GetDialog()->appendLog("Signature or public key are invalid, please enter valid signature/public key to verify message!");
        return false;
    }

    string digest = SHA256(msg, len);

    const unsigned char** pubKeyVCpp = &pubKey;
    EC_KEY *curPubKey = EC_KEY_new_by_curve_name(OBJ_txt2nid(ECCTYPE));
    curPubKey = o2i_ECPublicKey(&curPubKey, pubKeyVCpp, (long)pubKeyLen);

    bool res = (1 == ECDSA_verify(0, (unsigned char*)digest.c_str(), (int)digest.size(), signature, signatureLen, curPubKey));

    EC_KEY_free(curPubKey);
    return res;
}
