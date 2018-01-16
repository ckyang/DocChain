//
//  dialog.h
//  DocChain
//
//  Created by Chung-kaiYang on 01/13/18.
//

#ifndef dialog_h
#define dialog_h

#include <QDialog>
#include <QThread>
#include <string>
#include <unordered_map>

using namespace std;

class QApplication;
class QScrollArea;
class QGridLayout;
class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class QPixmap;
class QPMovie;
class QTextEdit;
class dialog_controller;
class block;

class dialog : public QDialog
{
    Q_OBJECT
    QThread workerThread;

public:
    enum DIALOG_COMMAND_TYPE
    {
        DIALOG_COMMAND_TYPE_ADD = 0,
        DIALOG_COMMAND_TYPE_REMOVE = 1,
        DIALOG_COMMAND_TYPE_UPDATEALL = 2,
    };

    dialog(QWidget *parent = 0, QApplication* app = 0);
    ~dialog();

    //Command Format: [MODE][POSITION][Message]
    //MODE(1): "+", "-", "*": Indicate add(+), remove(-), or updateAll(*)
    //POSITION(4): (0000 ~ FFFF) Indicate add/remove location
    //Message(-): Add/remove/all message
    static string GetCommand(DIALOG_COMMAND_TYPE type, int position, string msg);
    static void RetrieveCommand(const string& command, DIALOG_COMMAND_TYPE& type, int& position, string& msg);

signals:
    void appendLog(const QString& log);
    void updateRemoteDoc(const QString& command);
    void accumulateValidation(const QString& hash);
    void updateAddress(const QString& address);

// Below functions are Qt internal only, don't call them directly.
public slots:
    void handleAppendLog(const QString& log);
    void handleUpdateRemoteDoc(const QString& command);
    void handleAccumulateValidation(const QString& hash);
    void handleUpdateAddress(const QString& address);

private slots:
    void UIModifyDoc();

private:
    QLabel *m_blockChainTitleLabel, *m_logLabel;
    QTextEdit *m_docArea;
    QApplication *m_app;
    QGridLayout *m_mainLayout;
    QVBoxLayout *m_blockChainListLayout;
    QScrollArea *m_logScrollArea;
    QMovie *m_loadingMovie;
    QPixmap *m_tickPix;
    unordered_map<string, pair<int, block*>> m_validatingBlockHash;
    int m_preDocSize;

    dialog_controller *m_controller;
};

class dialog_controller : public QObject
{
    Q_OBJECT
    QThread workerThread;
    
public slots:
    void operateAppendLog(const QString& log);
    void operateUpdateRemoteDoc(const QString& command);
    void operateAccumulateValidation(const QString& hash);
    void operateUpdateAddress(const QString& address);

signals:
    void resultReadyAppendLog(const QString& log);
    void resultReadyUpdateRemoteDoc(const QString& command);
    void resultReadyAccumulateValidation(const QString& hash);
    void resultReadyUpdateAddress(const QString& address);
};

#endif /* dialog_h */
