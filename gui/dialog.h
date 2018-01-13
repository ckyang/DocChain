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
    dialog(QWidget *parent = 0, QApplication* app = 0);
    ~dialog();

signals:
    void appendLog(const QString& log);
    //Need to be changed to updateDoc(const QString& message);
    void updateBlockChainList();
    void accumulateValidation(const QString& hash);
    void updateAddress(const QString& address);

// Below functions are Qt internal only, don't call them directly.
public slots:
    void handleAppendLog(const QString &);
    void handleUpdateBlockChainList();
    void handleAccumulateValidation(const QString&);
    void handleUpdateAddress(const QString& address);

private slots:
    void modifyDoc();

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

    dialog_controller *m_controller;
};

class dialog_controller : public QObject
{
    Q_OBJECT
    QThread workerThread;
    
public slots:
    void operateAppendLog(const QString& log);
    void operateUpdateBlockChainList();
    void operateAccumulateValidation(const QString& hash);
    void operateUpdateAddress(const QString& address);

signals:
    void resultReadyAppendLog(const QString& log);
    void resultReadyUpdateBlockChainList();
    void resultReadyAccumulateValidation(const QString& hash);
    void resultReadyUpdateAddress(const QString& address);
};

#endif /* dialog_h */
