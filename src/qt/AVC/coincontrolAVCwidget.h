// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2020 The AVC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COINCONTROLAVCWIDGET_H
#define COINCONTROLAVCWIDGET_H

#include <QDialog>

namespace Ui {
class CoinControlAVCWidget;
}

class CoinControlAVCWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CoinControlAVCWidget(QWidget *parent = nullptr);
    ~CoinControlAVCWidget();

private:
    Ui::CoinControlAVCWidget *ui;
};

#endif // COINCONTROLAVCWIDGET_H
