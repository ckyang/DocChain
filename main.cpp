//
//  main.cpp
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//  Copyright © 2018 Chung-kaiYang. All rights reserved.
//

#include <QtWidgets/QApplication>
#include "block.h"
#include "factory.h"
#include "blockChain.h"
#include "talk.h"
#include "crypto.h"
#include "dialog.h"

using namespace std;

enum USER_COMMAND
{
    USER_COMMAND_QUIT = 0,
    USER_COMMAND_ADD_BLOCK = 1,
    USER_COMMAND_LIST_BLOCKCHAIN = 2,
    USER_COMMAND_MAX
};

void* create_talk(void *talkClass)
{
    pthread_detach(pthread_self());
    ((talk*)talkClass)->connect();
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    dialog* dialog = factory::GetDialog(&app);
    dialog->show();

    pthread_t thread;
    talk* talkObject = factory::GetTalk();
    pthread_create(&thread, NULL , create_talk, (void*) talkObject);

    dialog->appendLog("Welcome to blockchain demo program @ Clark Yang!");
    factory::GetBlockChain();

    talk::BroadcastPublicKey();
    talk::Broadcast(REMOTE_COMMAND_GET_PUBKEY);
    talk::Broadcast(REMOTE_COMMAND_GET_ALL);

    return app.exec();
}

