//
//  factory.cpp
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#include "factory.h"
#include "blockChain.h"
#include "talk.h"
#include "crypto.h"
#include "dialog.h"

blockChain* factory::m_blockChain = NULL;
talk* factory::m_talk = NULL;
crypto* factory::m_crypto = NULL;
dialog* factory::m_dialog = NULL;

blockChain* factory::GetBlockChain()
{
    if(!m_blockChain)
        m_blockChain = new blockChain(true);

    return m_blockChain;
}

talk* factory::GetTalk()
{
    if(!m_talk)
        m_talk = new talk();
    
    return m_talk;
}

crypto* factory::GetCrypto()
{
    if(!m_crypto)
        m_crypto = new crypto();
    
    return m_crypto;
}

dialog* factory::GetDialog(QApplication *app)
{
    if(!m_dialog)
        m_dialog = new dialog(0, app);
    
    return m_dialog;
}

factory::~factory(void)
{
    if(m_blockChain)
    {
        delete m_blockChain;
        m_blockChain = NULL;
    }

    if(m_talk)
    {
        delete m_talk;
        m_talk = NULL;
    }

    if(m_crypto)
    {
        delete m_crypto;
        m_crypto = NULL;
    }

    if(m_dialog)
    {
        delete m_dialog;
        m_dialog = NULL;
    }
}
