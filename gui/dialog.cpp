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

dialog::dialog(QWidget *parent, QApplication* app)
:QDialog(parent), m_app(app)
{
    assert(m_app);
    m_preDocSize = 0;
    m_mainLayout = new QGridLayout(this);
    m_loadingMovie = new QMovie("/Users/CK-Yang/Qt/image/loading.gif");
//    m_tickPix = new QPixmap("/Users/CK-Yang/Qt/image/tick.png");
//    m_loadingMovie = new QMovie(":/image/loading.gif");
    m_tickPix = new QPixmap(QCoreApplication::applicationDirPath() + "/tick.png");

    m_blockChainListLayout = new QVBoxLayout(this);
    m_blockChainTitleLabel = new QLabel("Address: []");
    m_docArea = new QTextEdit("");
    m_docArea->setStyleSheet("color:white; background-color:#995599; font:italic");
    m_blockChainListLayout->addWidget(m_blockChainTitleLabel);
    m_blockChainListLayout->addWidget(m_docArea);
    m_blockChainListLayout->setStretch(0, 1);
    m_blockChainListLayout->setStretch(1, 10);
    connect(m_docArea, SIGNAL(textChanged()), this, SLOT(modifyUIDoc()));

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
    connect(this, SIGNAL(updateRemoteDoc(QString)), m_controller, SLOT(operateUpdateRemoteDoc(QString)));
    connect(m_controller, SIGNAL(resultReadyUpdateRemoteDoc(QString)), this, SLOT(handleUpdateRemoteDoc(QString)));
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

void dialog::modifyUIDoc()
{
    if(m_docArea->toPlainText().length() == m_preDocSize)
        return;

    string message = GetCommand((m_docArea->toPlainText().length() > m_preDocSize) ? DIALOG_COMMAND_TYPE_ADD : DIALOG_COMMAND_TYPE_REMOVE, m_docArea->textCursor().position(), string(1, m_docArea->toPlainText().toUtf8().constData()[m_docArea->textCursor().position() - 1]));

    blockChain* blockchain = factory::GetBlockChain();
    blockchain->addBlock(blockchain->generateNextBlock(message));
    talk::Broadcast(string(REMOTE_COMMAND_NEW) + " " + blockchain->getLatestBlock()->getBlockInfo());
    m_preDocSize = m_docArea->toPlainText().length();

    resumeOriginalColor();
}

void dialog::handleUpdateRemoteDoc(const QString& command)
{
    DIALOG_COMMAND_TYPE type;
    int position;
    string msg;
    static QTextCharFormat addFormat;
    addFormat.setForeground(QBrush(QColor("red")));

    RetrieveCommand(command.toUtf8().constData(), type, position, msg);
    resumeOriginalColor();

    bool bOldState = m_docArea->blockSignals(true);

    switch(type)
    {
        case DIALOG_COMMAND_TYPE_ADD:
        {
            QTextCursor cursor = m_docArea->textCursor();
            cursor.setPosition(position - 1);
            m_docArea->setTextCursor(cursor);
            cursor.setCharFormat(addFormat);
            cursor.insertText(msg.c_str());
            break;
        }

        case DIALOG_COMMAND_TYPE_REMOVE:
        {
            QTextCursor cursor = m_docArea->textCursor();
            cursor.setPosition(position + 1);
            m_docArea->setTextCursor(cursor);
            m_docArea->textCursor().deletePreviousChar();
            break;
        }
            
        case DIALOG_COMMAND_TYPE_UPDATEALL:
        {
            m_docArea->setText(msg.c_str());
            break;
        }
            
        default:
            break;
    }

    m_docArea->blockSignals(bOldState);
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

string dialog::GetCommand(DIALOG_COMMAND_TYPE type, int position, string msg)
{
    static char message[10], lenHex[4] = {'0'};
    memset(message, '\0', 10);
    talk::ItoHex(position, lenHex);
    memcpy(message + 1, lenHex, 4);

    switch(type)
    {
        case DIALOG_COMMAND_TYPE_ADD:
        {
            message[0] = '+';
            break;
        }

        case DIALOG_COMMAND_TYPE_REMOVE:
        {
            message[0] = '-';
            break;
        }

        case DIALOG_COMMAND_TYPE_UPDATEALL:
        {
            message[0] = '*';
            break;
        }
        
        default:
            break;
    }

    return string(message) + msg;
}

void dialog::RetrieveCommand(const string& command, DIALOG_COMMAND_TYPE& type, int& position, string& msg)
{
    switch(command[0])
    {
        case '+':
        {
            type = DIALOG_COMMAND_TYPE_ADD;
            break;
        }
        case '-':
        {
            type = DIALOG_COMMAND_TYPE_REMOVE;
            break;
        }
        case '*':
        {
            type = DIALOG_COMMAND_TYPE_UPDATEALL;
            break;
        }
        default:
            break;
    }

    talk::HextoI(position, command.substr(1, 4).c_str());
    msg = command.substr(5);
}

void dialog::resumeOriginalColor()
{
    bool bOldState = m_docArea->blockSignals(true);
    static QTextCharFormat oriFormat;
    oriFormat.setForeground(QBrush(QColor("white")));
    QTextCursor cursor = m_docArea->textCursor();
    int oldPos = cursor.position();
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, m_docArea->toPlainText().length());
    cursor.setCharFormat(oriFormat);
    cursor.clearSelection();
    cursor.setPosition(oldPos);
    m_docArea->setTextCursor(cursor);
    m_docArea->blockSignals(bOldState);
}

void dialog_controller::operateAppendLog(const QString& log)
{
    emit resultReadyAppendLog(log);
}

void dialog_controller::operateUpdateRemoteDoc(const QString& command)
{
    emit resultReadyUpdateRemoteDoc(command);
}

void dialog_controller::operateUpdateAddress(const QString& address)
{
    emit resultReadyUpdateAddress(address);
}
