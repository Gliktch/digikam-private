/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-07-03
 * Description : dialog to edit and create digiKam xmp namespaces
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QString>
#include <QKeySequence>
#include <QDialog>

// Local includes

#include "dmetadatasettingscontainer.h"

namespace Digikam
{

class NamespaceEditDlg : public QDialog
{
    Q_OBJECT

public:

    explicit NamespaceEditDlg(bool create,
                              NamespaceEntry& entry,
                              QWidget* const parent = nullptr);
    ~NamespaceEditDlg() override;

    void saveData(NamespaceEntry& entry);

    static bool create(QWidget* const parent, NamespaceEntry& entry);
    static bool edit(QWidget* const parent, NamespaceEntry& entry);

public Q_SLOTS:

    void accept()       override;

private Q_SLOTS:

    void slotHelp();

private:

    void setupTagGui(NamespaceEntry& entry);
    void populateFields(NamespaceEntry& entry);
    void setType(NamespaceEntry::NamespaceType type);
    void makeReadOnly();
    bool validifyCheck(QString& errMsg);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
