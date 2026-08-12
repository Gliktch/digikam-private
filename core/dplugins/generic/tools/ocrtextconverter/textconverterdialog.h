/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : Text converter batch dialog
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QTreeWidgetItem>

// Local includes

#include "dplugindialog.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class TextConverterActionData;

class TextConverterDialog : public DPluginDialog
{

    Q_OBJECT

public:

    explicit TextConverterDialog(QWidget* const parent, DInfoInterface* const iface);
    ~TextConverterDialog()                                    override;

   void addItems(const QList<QUrl>& itemList);

protected:

    void closeEvent(QCloseEvent* e);

private:

    void setBusy(bool busy);

    void processAll();
    void processingFailed(const QUrl& url, int result);
    void processed(const QUrl& url, const QString& outputFile, const QString& ocrResult);

    int  calculateNumberOfWords(const QString& text)    const;

    void plugProcessMenu();
    void unplugProcessMenu();

private Q_SLOTS:

    void slotDefault();
    void slotClose();
    void slotStartStop();
    void slotAborted();
    void slotThreadFinished();

    void slotTextConverterAction(const DigikamGenericTextConverterPlugin::TextConverterActionData&);
    void slotDoubleClick(QTreeWidgetItem* element);
    void slotUpdateText();

    void slotSetDisable();

    void slotStartFoundTesseract();
    void slotTesseractBinaryFound(bool);
    void slotProcessMenu();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericTextConverterPlugin
