// Copyright (c) 2017-2019 The PIVX developers
// Copyright (c) 2020 The AVC developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZAVCCONTROLDIALOG_H
#define ZAVCCONTROLDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "zAVC/zerocoin.h"

class CZerocoinMint;
class WalletModel;

namespace Ui {
class ZAVCControlDialog;
}

class CZAVCControlWidgetItem : public QTreeWidgetItem
{
public:
    explicit CZAVCControlWidgetItem(QTreeWidget *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    explicit CZAVCControlWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    explicit CZAVCControlWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {}

    bool operator<(const QTreeWidgetItem &other) const;
};

class ZAVCControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZAVCControlDialog(QWidget *parent);
    ~ZAVCControlDialog();

    void setModel(WalletModel* model);

    static std::set<std::string> setSelectedMints;
    static std::set<CMintMeta> setMints;
    static std::vector<CMintMeta> GetSelectedMints();

private:
    Ui::ZAVCControlDialog *ui;
    WalletModel* model;

    void updateList();
    void updateLabels();

    enum {
        COLUMN_CHECKBOX,
        COLUMN_DENOMINATION,
        COLUMN_PUBCOIN,
        COLUMN_VERSION,
        COLUMN_CONFIRMATIONS,
        COLUMN_ISSPENDABLE
    };
    friend class CZAVCControlWidgetItem;

private slots:
    void updateSelection(QTreeWidgetItem* item, int column);
    void ButtonAllClicked();
};

#endif // ZAVCCONTROLDIALOG_H
