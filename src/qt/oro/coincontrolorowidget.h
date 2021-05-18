// Copyright (c) 2019 The ORO developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COINCONTROLOROWIDGET_H
#define COINCONTROLOROWIDGET_H

#include <QDialog>

namespace Ui {
class CoinControlOroWidget;
}

class CoinControlOroWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CoinControlOroWidget(QWidget *parent = nullptr);
    ~CoinControlOroWidget();

private:
    Ui::CoinControlOroWidget *ui;
};

#endif // COINCONTROLOROWIDGET_H
