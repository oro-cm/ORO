// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018-2019 The ORO developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "bitcoinunits.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "init.h"
#include "obfuscation.h"
#include "obfuscationconfig.h"
#include "optionsmodel.h"
#include "transactionfilterproxy.h"
#include "transactionrecord.h"
#include "transactiontablemodel.h"
#include "walletmodel.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QSettings>
#include <QTimer>

#define DECORATION_SIZE 48
#define ICON_OFFSET 16
#define NUM_ITEMS 9

extern CWallet* pwalletMain;

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate() : QAbstractItemDelegate(), unit(BitcoinUnits::ORO)
    {
    }

    inline void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        mainRect.moveLeft(ICON_OFFSET);
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2 * ypad) / 2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top() + ypad, mainRect.width() - xspace - ICON_OFFSET, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top() + ypad + halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();

        // Check transaction status
        int nStatus = index.data(TransactionTableModel::StatusRole).toInt();
        bool fConflicted = false;
        if (nStatus == TransactionStatus::Conflicted || nStatus == TransactionStatus::NotAccepted) {
            fConflicted = true; // Most probably orphaned, but could have other reasons as well
        }
        bool fImmature = false;
        if (nStatus == TransactionStatus::Immature) {
            fImmature = true;
        }

        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = COLOR_BLACK;
        if (value.canConvert<QBrush>()) {
            QBrush brush = qvariant_cast<QBrush>(value);
            foreground = brush.color();
        }

        painter->setPen(foreground);
        QRect boundingRect;
        painter->drawText(addressRect, Qt::AlignLeft | Qt::AlignVCenter, address, &boundingRect);

        if (index.data(TransactionTableModel::WatchonlyRole).toBool()) {
            QIcon iconWatchonly = qvariant_cast<QIcon>(index.data(TransactionTableModel::WatchonlyDecorationRole));
            QRect watchonlyRect(boundingRect.right() + 5, mainRect.top() + ypad + halfheight, 16, halfheight);
            iconWatchonly.paint(painter, watchonlyRect);
        }

        if (fConflicted) { // No need to check anything else for conflicted transactions
            foreground = COLOR_CONFLICTED;
        } else if (!confirmed || fImmature) {
            foreground = COLOR_UNCONFIRMED;
        } else if (amount < 0) {
            foreground = COLOR_NEGATIVE;
        } else {
            foreground = COLOR_BLACK;
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true, BitcoinUnits::separatorComma);
        if (!confirmed) {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight | Qt::AlignVCenter, amountText);

        painter->setPen(COLOR_BLACK);
        painter->drawText(amountRect, Qt::AlignLeft | Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;
};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget* parent) : QWidget(parent),
                                              ui(new Ui::OverviewPage),
                                              clientModel(0),
                                              walletModel(0),
                                              currentBalance(-1),
                                              currentUnconfirmedBalance(-1),
                                              currentImmatureBalance(-1),
                                              currentZerocoinBalance(-1),
                                              currentUnconfirmedZerocoinBalance(-1),
                                              currentimmatureZerocoinBalance(-1),
                                              currentWatchOnlyBalance(-1),
                                              currentWatchUnconfBalance(-1),
                                              currentWatchImmatureBalance(-1),
                                              txdelegate(new TxViewDelegate()),
                                              filter(0)
{
    nDisplayUnit = 0; // just make sure it's not unitialized
    ui->setupUi(this);

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");

    ui->label_5z_3->hide();
    ui->labelzOROPercent->hide();
    ui->labelzBalanceMatureText->hide();
    ui->labelzBalanceMature->hide();
    ui->labelzBalanceUnconfirmedText->hide();
    ui->labelzBalanceUnconfirmed->hide();
    ui->labelzBalanceImmatureText->hide();
    ui->labelzBalanceImmature->hide();
    ui->labelzBalanceText->hide();
    ui->labelzBalance->hide();
    ui->line_ZerocoinBalance->hide();
    ui->labelzOROPercent->hide();

    m_nBlockHeight = 0;

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(setIssuedCoins(QNetworkReply*)));

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
}

void OverviewPage::handleTransactionClicked(const QModelIndex& index)
{
    if (filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::getPercentage(CAmount nUnlockedBalance, CAmount nZerocoinBalance, QString& sOROPercentage, QString& szOROPercentage)
{
    int nPrecision = 2;
    double dzPercentage = 0.0;

    if (nZerocoinBalance <= 0) {
        dzPercentage = 0.0;
    } else {
        if (nUnlockedBalance <= 0) {
            dzPercentage = 100.0;
        } else {
            dzPercentage = 100.0 * (double)(nZerocoinBalance / (double)(nZerocoinBalance + nUnlockedBalance));
        }
    }

    double dPercentage = 100.0 - dzPercentage;

    szOROPercentage = "(" + QLocale(QLocale::system()).toString(dzPercentage, 'f', nPrecision) + " %)";
    sOROPercentage = "(" + QLocale(QLocale::system()).toString(dPercentage, 'f', nPrecision) + " %)";
}

void OverviewPage::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, const CAmount& zerocoinBalance, const CAmount& unconfirmedZerocoinBalance, const CAmount& immatureZerocoinBalance, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
    currentBalance = balance;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    currentZerocoinBalance = zerocoinBalance;
    currentUnconfirmedZerocoinBalance = unconfirmedZerocoinBalance;
    currentimmatureZerocoinBalance = immatureZerocoinBalance;
    currentWatchOnlyBalance = watchOnlyBalance;
    currentWatchUnconfBalance = watchUnconfBalance;
    currentWatchImmatureBalance = watchImmatureBalance;

    CAmount nLockedBalance = 0;
    CAmount nWatchOnlyLockedBalance = 0;
    if (pwalletMain) {
        nLockedBalance = pwalletMain->GetLockedCoins();
        nWatchOnlyLockedBalance = pwalletMain->GetLockedWatchOnlyBalance();
    }
    // ORO Balance
    CAmount nTotalBalance = balance + unconfirmedBalance + nLockedBalance;
    CAmount pivAvailableBalance = balance - immatureBalance;
    CAmount nTotalWatchBalance = watchOnlyBalance + watchUnconfBalance + watchImmatureBalance;
    CAmount nUnlockedBalance = nTotalBalance - nLockedBalance - nLockedBalance; // increment nLockedBalance twice because it was added to
                                                                                // nTotalBalance above
    // zORO Balance
    CAmount matureZerocoinBalance = zerocoinBalance - unconfirmedZerocoinBalance - immatureZerocoinBalance;
    // Percentages
    QString szPercentage = "";
    QString sPercentage = "";
    getPercentage(nUnlockedBalance, zerocoinBalance, sPercentage, szPercentage);
    // Combined balances
    CAmount availableTotalBalance = pivAvailableBalance + matureZerocoinBalance;
    CAmount sumTotalBalance = nTotalBalance + zerocoinBalance;

    // ORO labels
    ui->labelBalance->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, pivAvailableBalance, false, BitcoinUnits::separatorComma));
    ui->labelUnconfirmed->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, unconfirmedBalance, false, BitcoinUnits::separatorComma));
    ui->labelImmature->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, immatureBalance, false, BitcoinUnits::separatorComma));
    ui->labelLockedBalance->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, nLockedBalance, false, BitcoinUnits::separatorComma));
    ui->labelTotal->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, nTotalBalance, false, BitcoinUnits::separatorComma));

    // Watchonly labels
    ui->labelWatchAvailable->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchOnlyBalance, false, BitcoinUnits::separatorComma));
    ui->labelWatchPending->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchUnconfBalance, false, BitcoinUnits::separatorComma));
    ui->labelWatchImmature->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchImmatureBalance, false, BitcoinUnits::separatorComma));
    ui->labelWatchLocked->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, nWatchOnlyLockedBalance, false, BitcoinUnits::separatorComma));
    ui->labelWatchTotal->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, nTotalWatchBalance, false, BitcoinUnits::separatorComma));

    // zORO labels
    ui->labelzBalance->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, zerocoinBalance, false, BitcoinUnits::separatorComma));
    ui->labelzBalanceUnconfirmed->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, unconfirmedZerocoinBalance, false, BitcoinUnits::separatorComma));
    ui->labelzBalanceMature->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, matureZerocoinBalance, false, BitcoinUnits::separatorComma));
    ui->labelzBalanceImmature->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, immatureZerocoinBalance, false, BitcoinUnits::separatorComma));

    // Combined labels
    ui->labelBalancez->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, availableTotalBalance, false, BitcoinUnits::separatorComma));
    ui->labelTotalz->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, sumTotalBalance, false, BitcoinUnits::separatorComma));

    // Percentage labels
    ui->labelOROPercent->setText(sPercentage);
    // ui->labelzOROPercent->setText(szPercentage);

    // Adjust bubble-help according to AutoMint settings
    QString automintHelp = tr("Current percentage of zORO.\nIf AutoMint is enabled this percentage will settle around the configured AutoMint percentage (default = 10%).\n");
    bool fEnableZeromint = GetBoolArg("-enablezeromint", false);
    int nZeromintPercentage = GetArg("-zeromintpercentage", 10);
    if (fEnableZeromint) {
        automintHelp += tr("AutoMint is currently enabled and set to ") + QString::number(nZeromintPercentage) + "%.\n";
        automintHelp += tr("To disable AutoMint add 'enablezeromint=0' in oro.conf.");
    } else {
        automintHelp += tr("AutoMint is currently disabled.\nTo enable AutoMint change 'enablezeromint=0' to 'enablezeromint=1' in oro.conf");
    }

    // Only show most balances if they are non-zero for the sake of simplicity
    QSettings settings;
    bool settingShowAllBalances = !settings.value("fHideZeroBalances").toBool();
    bool showSumAvailable = settingShowAllBalances || sumTotalBalance != availableTotalBalance;
    ui->labelBalanceTextz->setVisible(showSumAvailable);
    ui->labelBalancez->setVisible(showSumAvailable);
    bool showOROAvailable = settingShowAllBalances || pivAvailableBalance != nTotalBalance;
    bool showWatchOnlyOROAvailable = watchOnlyBalance != nTotalWatchBalance;
    bool showOROPending = settingShowAllBalances || unconfirmedBalance != 0;
    bool showWatchOnlyOROPending = watchUnconfBalance != 0;
    bool showOROLocked = settingShowAllBalances || nLockedBalance != 0;
    bool showWatchOnlyOROLocked = nWatchOnlyLockedBalance != 0;
    bool showImmature = settingShowAllBalances || immatureBalance != 0;
    bool showWatchOnlyImmature = watchImmatureBalance != 0;
    bool showWatchOnly = nTotalWatchBalance != 0;
    ui->labelBalance->setVisible(showOROAvailable || showWatchOnlyOROAvailable);
    ui->labelBalanceText->setVisible(showOROAvailable || showWatchOnlyOROAvailable);
    ui->labelWatchAvailable->setVisible(showOROAvailable && showWatchOnly);
    ui->labelUnconfirmed->setVisible(showOROPending || showWatchOnlyOROPending);
    ui->labelPendingText->setVisible(showOROPending || showWatchOnlyOROPending);
    ui->labelWatchPending->setVisible(showOROPending && showWatchOnly);
    ui->labelLockedBalance->setVisible(showOROLocked || showWatchOnlyOROLocked);
    ui->labelLockedBalanceText->setVisible(showOROLocked || showWatchOnlyOROLocked);
    ui->labelWatchLocked->setVisible(showOROLocked && showWatchOnly);
    ui->labelImmature->setVisible(showImmature || showWatchOnlyImmature); // for symmetry reasons also show immature label when the watch-only one is shown
    ui->labelImmatureText->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelWatchImmature->setVisible(showImmature && showWatchOnly); // show watch-only immature balance
    bool showzOROAvailable = settingShowAllBalances || zerocoinBalance != matureZerocoinBalance;
    bool showzOROnconfirmed = settingShowAllBalances || unconfirmedZerocoinBalance != 0;
    bool showzOROImmature = settingShowAllBalances || immatureZerocoinBalance != 0;
    ui->labelzBalanceMature->setVisible(showzOROAvailable);
    ui->labelzBalanceMatureText->setVisible(showzOROAvailable);
    ui->labelzBalanceUnconfirmed->setVisible(showzOROnconfirmed);
    ui->labelzBalanceUnconfirmedText->setVisible(showzOROnconfirmed);
    ui->labelzBalanceImmature->setVisible(showzOROImmature);
    ui->labelzBalanceImmatureText->setVisible(showzOROImmature);
    bool showPercentages = !(zerocoinBalance == 0 && nTotalBalance == 0);
    ui->labelOROPercent->setVisible(showPercentages);
    // ui->labelzOROPercent->setVisible(showPercentages);

    static int cachedTxLocks = 0;

    if (cachedTxLocks != nCompleteTXLocks) {
        cachedTxLocks = nCompleteTXLocks;
        ui->listTransactions->update();
    }
}

// show/hide watch-only labels
void OverviewPage::updateWatchOnlyLabels(bool showWatchOnly)
{
    ui->labelSpendable->setVisible(showWatchOnly);      // show spendable label (only when watch-only is active)
    ui->labelWatchonly->setVisible(showWatchOnly);      // show watch-only label
    ui->labelWatchAvailable->setVisible(showWatchOnly); // show watch-only available balance
    ui->labelWatchPending->setVisible(showWatchOnly);   // show watch-only pending balance
    ui->labelWatchLocked->setVisible(showWatchOnly);    // show watch-only total balance
    ui->labelWatchTotal->setVisible(showWatchOnly);     // show watch-only total balance

    if (!showWatchOnly) {
        ui->labelWatchImmature->hide();
    } else {
        ui->labelBalance->setIndent(20);
        ui->labelUnconfirmed->setIndent(20);
        ui->labelLockedBalance->setIndent(20);
        ui->labelImmature->setIndent(20);
        ui->labelTotal->setIndent(20);
    }
}

void OverviewPage::setOROInfo(const CAmount& blockReward, const CAmount& coinPrice, const CAmount& targetPrice, const CAmount& totalCoins, const CAmount& issuedCoins, const CAmount& marketCap)
{
    int unit = walletModel->getOptionsModel()->getDisplayUnit();

    // currentBlockReward = blockReward;
    // currentTargetPrice = targetPrice;
    // currentTotalCoins = totalCoins;
    // currentIssuedCoins = issuedCoins;
    // currentMarketCap = marketCap;

    ui->labelBlockReward->setText(BitcoinUnits::formatWithUnit(unit, 1, false, BitcoinUnits::separatorComma));
    ui->labelCoinPrice->setText(USDUnits::formatWithUnit(unit, 1000, false, USDUnits::separatorComma, false, false));
    ui->labelTotalCoins->setText(BitcoinUnits::formatWithUnit(unit, totalCoins, false, BitcoinUnits::separatorComma));

    QNetworkRequest request(QUrl("http://oro.cm/circulation/"));
    currentReply = networkManager->get(request); // GET
}

void OverviewPage::setIssuedCoins(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
        return; // ...only in a blog post

    QByteArray data = reply->readAll();

    bool ok;
    int64_t issuedCoins = data.toLong(&ok, 10);
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    ui->labelIssuedCoins->setText(BitcoinUnits::formatWithUnit(unit, issuedCoins * 1000, false, BitcoinUnits::separatorComma));
    ui->labelMarketCap->setText(USDUnits::formatWithUnit(unit, issuedCoins * 1000, false, USDUnits::separatorComma, true, true));
}

void OverviewPage::setNumBlocks(int count)
{
    CAmount blockNumber = count;
    QString strBlockNumber = USDUnits::format(1, blockNumber * 10, false, USDUnits::separatorComma, true, true);
    ui->labelBlockNumber->setText(strBlockNumber);
}

void OverviewPage::setClientModel(ClientModel* model)
{
    this->clientModel = model;
    setNumBlocks(model->getNumBlocks());
    connect(model, SIGNAL(numBlocksChanged(int)), this, SLOT(setNumBlocks(int)));

    if (model) {
        // Show warning if this is a prerelease version
        connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
        updateAlerts(model->getStatusBarWarnings());
    }
    this->m_nBlockHeight = clientModel->getNumBlocks();
}

void OverviewPage::setWalletModel(WalletModel* model)
{
    this->walletModel = model;
    if (model && model->getOptionsModel()) {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getUnconfirmedBalance(), model->getImmatureBalance(),
            model->getZerocoinBalance(), model->getUnconfirmedZerocoinBalance(), model->getImmatureZerocoinBalance(),
            model->getWatchBalance(), model->getWatchUnconfirmedBalance(), model->getWatchImmatureBalance());
        connect(model, SIGNAL(balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)), this,
            SLOT(setBalance(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
        connect(model->getOptionsModel(), SIGNAL(hideZeroBalancesChanged(bool)), this, SLOT(updateDisplayUnit()));

        updateWatchOnlyLabels(model->haveWatchOnly());
        connect(model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyLabels(bool)));
        setOROInfo(model->getBlockReward(m_nBlockHeight), 0, 0,
            model->getTotalCoins(m_nBlockHeight), model->getIssuedCoins(), model->getMarketCap());
        connect(model, SIGNAL(oroInfoChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)), this, SLOT(setOROInfo(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)));
    }

    // update the display unit, to not use the default ("ORO")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if (currentBalance != -1)
            setBalance(currentBalance, currentUnconfirmedBalance, currentImmatureBalance, currentZerocoinBalance, currentUnconfirmedZerocoinBalance, currentimmatureZerocoinBalance,
                currentWatchOnlyBalance, currentWatchUnconfBalance, currentWatchImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = nDisplayUnit;

        ui->listTransactions->update();
    }
}

void OverviewPage::updateAlerts(const QString& warnings)
{
    this->ui->labelAlerts->setVisible(!warnings.isEmpty());
    this->ui->labelAlerts->setText(warnings);
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
    ui->labelTransactionsStatus->setVisible(fShow);
}
