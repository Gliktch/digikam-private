/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QPrinter>
#include <QList>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// Local includes

#include "dwizardpage.h"
#include "ditemslist.h"
#include "advprintphoto.h"
#include "ui_advprintphotopage.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class TemplateIcon;

class AdvPrintPhotoPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit AdvPrintPhotoPage(QWizard* const wizard, const QString& title);
    ~AdvPrintPhotoPage()                            override;

    QPrinter*             printer()           const;
    DItemsList*           imagesList()        const;
    Ui_AdvPrintPhotoPage* ui()                const;
    bool                  isComplete()        const override;
    int                   getPageCount()      const;

    void initializePage()                           override;
    bool validatePage()                             override;

    /**
     * Create a MxN grid of photos, fitting on the page.
     */
    void createPhotoGrid(AdvPrintPhotoSize* const p,
                         int pageWidth,
                         int pageHeight,
                         int rows,
                         int columns,
                         TemplateIcon* const iconpreview);

    void manageBtnPreviewPage();

    /**
     * Initialize page layout to the given pageSize in mm.
     */
    void initPhotoSizes(const QSizeF& pageSize);

private:

    /**
     * To parse template file with 'fn' as filename, and 'pageSize' in mm.
     */
    void parseTemplateFile(const QString& fn,
                           const QSizeF& pageSize);

public Q_SLOTS:

    void slotOutputChanged(const QString&);

private Q_SLOTS:

    void slotXMLLoadElement(QXmlStreamReader&);

    /**
     * Save item list => we catch the signal to add
     * our PA attributes and elements Image children
     */
    void slotXMLSaveItem(QXmlStreamWriter&, int);

    /**
     * Save item list => we catch the signal to add
     * our PA elements (not per image)
     */
    void slotXMLCustomElement(QXmlStreamWriter&);

    void slotXMLCustomElement(QXmlStreamReader&);
    void slotContextMenuRequested();
    void slotIncreaseCopies();
    void slotDecreaseCopies();
    void slotAddItems(const QList<QUrl>&);
    void slotRemovingItems(const QList<int>&);
    void slotBtnPrintOrderDownClicked();
    void slotBtnPrintOrderUpClicked();
    void slotBtnPreviewPageDownClicked();
    void slotBtnPreviewPageUpClicked();
    void slotListPhotoSizesSelected();
    void slotPageSetup();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPrintCreatorPlugin
