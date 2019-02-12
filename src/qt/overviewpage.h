// Copyright (c) 2011-2013 The Bitcoin developers
// Copyright (c) 2017 The PIVX developers
// Copyright (c) 2018-2019 The ORO developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_OVERVIEWPAGE_H
#define BITCOIN_QT_OVERVIEWPAGE_H

#include "amount.h"

#include <QWidget>
#include <QtNetwork>

class ClientModel;
class TransactionFilterProxy;
class TxViewDelegate;
class WalletModel;

namespace Ui
{
class OverviewPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget* parent = 0);
    ~OverviewPage();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);
    void showOutOfSyncWarning(bool fShow);

public slots:
    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, 
                    const CAmount& zerocoinBalance, const CAmount& unconfirmedZerocoinBalance, const CAmount& immatureZerocoinBalance,
                    const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

    void setOROInfo(const CAmount& blockReward, const CAmount& coinPrice,
                    const CAmount& targetPrice, const CAmount& totalCoins,
                    const CAmount& issuedCoins, const CAmount& marketCap);

    void setIssuedCoins(QNetworkReply* reply);

    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count);

signals:
    void transactionClicked(const QModelIndex& index);

private:
    QTimer* timer;
    Ui::OverviewPage* ui;
    ClientModel* clientModel;
    WalletModel* walletModel;
    CAmount currentBalance;
    CAmount currentUnconfirmedBalance;
    CAmount currentImmatureBalance;
    CAmount currentZerocoinBalance;
    CAmount currentUnconfirmedZerocoinBalance;
    CAmount currentimmatureZerocoinBalance;
    CAmount currentWatchOnlyBalance;
    CAmount currentWatchUnconfBalance;
    CAmount currentWatchImmatureBalance;
    
    QNetworkAccessManager* networkManager;
    QNetworkReply* currentReply;

    int64_t m_nBlockHeight;

    int nDisplayUnit;
    void getPercentage(CAmount nTotalBalance, CAmount nZerocoinBalance, QString& sOROPercentage, QString& szOROPercentage);

    TxViewDelegate* txdelegate;
    TransactionFilterProxy* filter;

private slots:
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex& index);
    void updateAlerts(const QString& warnings);
    void updateWatchOnlyLabels(bool showWatchOnly);
};

#endif // BITCOIN_QT_OVERVIEWPAGE_H
