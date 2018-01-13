//
//  dialog.cpp
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//

#include <assert.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtGui/QPixmap>
#include <QtGui/QMovie>
#include "dialog.h"
#include "factory.h"
#include "blockchain.h"
#include "talk.h"
#include "block.h"
#include "crypto.h"

#define VELIDATION_NUMBER 2

dialog::dialog(QWidget *parent, QApplication* app)
:QDialog(parent), m_app(app)
{
    assert(m_app);
    m_mainLayout = new QGridLayout(this);
    m_loadingMovie = new QMovie("/Users/CK-Yang/Qt/image/loading.gif");
//    m_tickPix = new QPixmap("/Users/CK-Yang/Qt/image/tick.png");
//    m_loadingMovie = new QMovie(":/image/loading.gif");
    m_tickPix = new QPixmap(QCoreApplication::applicationDirPath() + "/tick.png");

//    connect(m_addBlockButton, SIGNAL(clicked()), this, SLOT(verifyBlock()));

    m_blockChainListLayout = new QVBoxLayout(this);
    m_blockChainTitleLabel = new QLabel("Address: []");
    m_docArea = new QTextEdit("Empty");
    m_docArea->setStyleSheet("color:white; background-color:#995599; font:italic");
    m_blockChainListLayout->addWidget(m_blockChainTitleLabel);
    m_blockChainListLayout->addWidget(m_docArea);
    m_blockChainListLayout->setStretch(0, 1);
    m_blockChainListLayout->setStretch(1, 10);
    
    m_logLabel = new QLabel("QT initialized.");
    m_logLabel->setStyleSheet("QLabel { background-color:transparent; color : lime; }");
    m_logScrollArea = new QScrollArea;
    m_logScrollArea->setStyleSheet("background-color:black;");
    m_logScrollArea->resize (5, 5);
    m_logScrollArea->setWidget(m_logLabel);

    m_mainLayout->addLayout(m_blockChainListLayout, 1, 0);
    m_mainLayout->addWidget(m_logScrollArea, 2, 0);
    m_mainLayout->setRowStretch(1, 8);
    m_mainLayout->setRowStretch(2, 2);

    m_controller = new dialog_controller();
    m_controller->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), m_controller, SLOT(deleteLater()));

    connect(this, SIGNAL(appendLog(QString)), m_controller, SLOT(operateAppendLog(QString)));
    connect(m_controller, SIGNAL(resultReadyAppendLog(QString)), this, SLOT(handleAppendLog(QString)));
    connect(this, SIGNAL(updateBlockChainList()), m_controller, SLOT(operateUpdateBlockChainList()));
    connect(m_controller, SIGNAL(resultReadyUpdateBlockChainList()), this, SLOT(handleUpdateBlockChainList()));
    connect(this, SIGNAL(accumulateValidation(QString)), m_controller, SLOT(operateAccumulateValidation(QString)));
    connect(m_controller, SIGNAL(resultReadyAccumulateValidation(QString)), this, SLOT(handleAccumulateValidation(QString)));
    connect(this, SIGNAL(updateAddress(QString)), m_controller, SLOT(operateUpdateAddress(QString)));
    connect(m_controller, SIGNAL(resultReadyUpdateAddress(QString)), this, SLOT(handleUpdateAddress(QString)));

    workerThread.start();
}

dialog::~dialog()
{
    delete(m_mainLayout);
    delete(m_blockChainListLayout);
    delete(m_blockChainTitleLabel);
    delete(m_docArea);
    delete(m_logLabel);
    delete(m_logScrollArea);
    delete(m_loadingMovie);
    delete(m_tickPix);
    workerThread.quit();
    workerThread.wait();
    delete(m_controller);
}

void dialog::verifyBlock()
{
/*
//    m_addBlockLabel->setMovie(m_loadingMovie);
//    m_loadingMovie->start();

    m_addBlockLabel->setText("Validating...");
    block* newBlock = factory::GetBlockChain()->generateNextBlock(m_addBlockNameEdit->text().toUtf8().constData());
    m_validatingBlockHash[newBlock->getHash()] = make_pair(0, newBlock);
    talk::Broadcast(string(REMOTE_COMMAND_ASK_VERIFY) + " " + newBlock->getBlockInfo());
    m_addBlockNameEdit->clear();
*/
}

void dialog::handleAccumulateValidation(const QString& hash)
{
    string curHash = hash.toUtf8().constData();
    if(m_validatingBlockHash.find(curHash) == m_validatingBlockHash.end())
        return;

    ++m_validatingBlockHash[curHash].first;

    if(VELIDATION_NUMBER == m_validatingBlockHash[curHash].first)
    {
        factory::GetBlockChain()->addBlock(m_validatingBlockHash[curHash].second);
        m_validatingBlockHash.erase(curHash);
        updateBlockChainList();
//        m_addBlockLabel->setText("OK!");
        talk::Broadcast(string(REMOTE_COMMAND_NEW) + " " + factory::GetBlockChain()->getLatestBlock()->getBlockInfo());
        //    m_addBlockLabel->setPixmap(*m_tickPix);
    }
//    else
//        m_addBlockLabel->setText(QString("+").append(to_string(m_validatingBlockHash[curHash].first).c_str()));
}

void dialog::handleUpdateBlockChainList()
{
    m_docArea->setText(factory::GetBlockChain()->getChainInfo(true).c_str());
    m_docArea->adjustSize();
//    m_blockChainScrollArea->widget()->resize(m_blockChainScrollArea->widget()->sizeHint());
    m_app->processEvents();
//    m_blockChainScrollArea->verticalScrollBar()->setValue(m_blockChainScrollArea->verticalScrollBar()->minimum());
//    m_blockChainScrollArea->horizontalScrollBar()->setValue(m_blockChainScrollArea->horizontalScrollBar()->minimum());
}

void dialog::handleAppendLog(const QString& log)
{
    m_logLabel->setText(m_logLabel->text().append("\n").append(log));
    m_logLabel->adjustSize();
    m_logScrollArea->widget()->resize(m_logScrollArea->widget()->sizeHint());
    m_app->processEvents();
    m_logScrollArea->verticalScrollBar()->setValue(m_logScrollArea->verticalScrollBar()->maximum());
    m_logScrollArea->horizontalScrollBar()->setValue(m_logScrollArea->horizontalScrollBar()->minimum());
}

void dialog::handleUpdateAddress(const QString& address)
{
    m_blockChainTitleLabel->setText(QString("Address: [").append(address).append("]"));
}

void dialog_controller::operateAppendLog(const QString& log)
{
    emit resultReadyAppendLog(log);
}

void dialog_controller::operateUpdateBlockChainList()
{
    emit resultReadyUpdateBlockChainList();
}

void dialog_controller::operateAccumulateValidation(const QString& hash)
{
    emit resultReadyAccumulateValidation(hash);
}

void dialog_controller::operateUpdateAddress(const QString& address)
{
    emit resultReadyUpdateAddress(address);
}
