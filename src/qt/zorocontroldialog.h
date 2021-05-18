// Copyright (c) 2017-2019 The ORO developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZOROCONTROLDIALOG_H
#define ZOROCONTROLDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "zoro/zerocoin.h"

class CZerocoinMint;
class WalletModel;

namespace Ui {
class ZOroControlDialog;
}

class CZOroControlWidgetItem : public QTreeWidgetItem
{
public:
    explicit CZOroControlWidgetItem(QTreeWidget *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    explicit CZOroControlWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    explicit CZOroControlWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {}

    bool operator<(const QTreeWidgetItem &other) const;
};

class ZOroControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZOroControlDialog(QWidget *parent);
    ~ZOroControlDialog();

    void setModel(WalletModel* model);

    static std::set<std::string> setSelectedMints;
    static std::set<CMintMeta> setMints;
    static std::vector<CMintMeta> GetSelectedMints();

private:
    Ui::ZOroControlDialog *ui;
    WalletModel* model;

    void updateList();
    void updateLabels();

    enum {
        COLUMN_CHECKBOX,
        COLUMN_DENOMINATION,
        COLUMN_PUBCOIN,
        COLUMN_VERSION,
        COLUMN_PRECOMPUTE,
        COLUMN_CONFIRMATIONS,
        COLUMN_ISSPENDABLE
    };
    friend class CZOroControlWidgetItem;

private slots:
    void updateSelection(QTreeWidgetItem* item, int column);
    void ButtonAllClicked();
};

#endif // ZOROCONTROLDIALOG_H
