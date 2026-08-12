/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a dialog to edit EXIF,IPTC and XMP metadata
 *
 * SPDX-FileCopyrightText: 2011      by Victor Dodon <dodon dot victor at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QCloseEvent>
#include <QUrl>

// Local includes

#include "dplugindialog.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class MetadataEditDialog : public DPluginDialog
{
    Q_OBJECT

public:

    explicit MetadataEditDialog(QWidget* const parent, DInfoInterface* const iface);
    ~MetadataEditDialog()                                      override;

    QList<QUrl>::iterator currentItem()                  const;
    QString currentItemTitleHeader(const QString& title) const;

Q_SIGNALS:

    void signalMetadataChangedForUrl(const QUrl&);

public Q_SLOTS:

    void slotModified();

private Q_SLOTS:

    void slotOk();
    void slotClose();
    void slotItemChanged();
    void slotApply();
    void slotNext();
    void slotPrevious();
    void slotSetReadOnly(bool);

protected:

    void closeEvent(QCloseEvent*)                              override;
    bool eventFilter(QObject*, QEvent*)                        override;

private:

    void saveSettings();
    void readSettings();
    void updatePreview();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
