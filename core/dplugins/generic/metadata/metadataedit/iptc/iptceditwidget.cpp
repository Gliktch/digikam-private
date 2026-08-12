/*============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a DConfigDlgWdg to edit IPTC metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011 by Victor Dodon <dodon dot victor at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iptceditwidget.h"

// Qt includes

#include <QMenu>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QUrl>
#include <QApplication>
#include <QFileInfo>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "metadataeditdialog.h"
#include "metaenginesettings.h"
#include "iptccategories.h"
#include "iptccontent.h"
#include "iptccredits.h"
#include "iptcenvelope.h"
#include "iptckeywords.h"
#include "iptcorigin.h"
#include "iptcproperties.h"
#include "iptcstatus.h"
#include "iptcsubjects.h"
#include "dmetadata.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN IPTCEditWidget::Private
{
public:

    Private() = default;

public:

    bool                  modified          = false;
    bool                  isReadOnly        = false;

    DConfigDlgWdgItem*    page_content      = nullptr;
    DConfigDlgWdgItem*    page_properties   = nullptr;
    DConfigDlgWdgItem*    page_subjects     = nullptr;
    DConfigDlgWdgItem*    page_keywords     = nullptr;
    DConfigDlgWdgItem*    page_categories   = nullptr;
    DConfigDlgWdgItem*    page_credits      = nullptr;
    DConfigDlgWdgItem*    page_status       = nullptr;
    DConfigDlgWdgItem*    page_origin       = nullptr;
    DConfigDlgWdgItem*    page_envelope     = nullptr;

    IPTCContent*          contentPage       = nullptr;
    IPTCProperties*       propertiesPage    = nullptr;
    IPTCSubjects*         subjectsPage      = nullptr;
    IPTCKeywords*         keywordsPage      = nullptr;
    IPTCCategories*       categoriesPage    = nullptr;
    IPTCCredits*          creditsPage       = nullptr;
    IPTCStatus*           statusPage        = nullptr;
    IPTCOrigin*           originPage        = nullptr;
    IPTCEnvelope*         envelopePage      = nullptr;

    MetadataEditDialog*   dlg               = nullptr;
};

IPTCEditWidget::IPTCEditWidget(MetadataEditDialog* const parent)
    : DConfigDlgWdg(parent),
      d            (new Private)
{
    d->dlg           = parent;

    d->contentPage   = new IPTCContent(this);
    d->page_content  = addPage(d->contentPage, i18nc("@item: iptc", "Content"));
    d->page_content->setIcon(QIcon::fromTheme(QLatin1String("draw-text")));

    d->originPage  = new IPTCOrigin(this);
    d->page_origin = addPage(d->originPage, i18nc("@item: iptc", "Origin"));
    d->page_origin->setIcon(QIcon::fromTheme(QLatin1String("globe")));

    d->creditsPage  = new IPTCCredits(this);
    d->page_credits = addPage(d->creditsPage, i18nc("@item: iptc", "Credits"));
    d->page_credits->setIcon(QIcon::fromTheme(QLatin1String("address-book-new")));

    d->subjectsPage  = new IPTCSubjects(this);
    d->page_subjects = addPage(d->subjectsPage, i18nc("@item: iptc", "Subjects"));
    d->page_subjects->setIcon(QIcon::fromTheme(QLatin1String("feed-subscribe")));

    d->keywordsPage  = new IPTCKeywords(this);
    d->page_keywords = addPage(d->keywordsPage, i18nc("@item: iptc", "Keywords"));
    d->page_keywords->setIcon(QIcon::fromTheme(QLatin1String("bookmark-new")));

    d->categoriesPage  = new IPTCCategories(this);
    d->page_categories = addPage(d->categoriesPage, i18nc("@item: iptc", "Categories"));
    d->page_categories->setIcon(QIcon::fromTheme(QLatin1String("folder-pictures")));

    d->statusPage  = new IPTCStatus(this);
    d->page_status = addPage(d->statusPage, i18nc("@item: iptc", "Status"));
    d->page_status->setIcon(QIcon::fromTheme(QLatin1String("view-task")));

    d->propertiesPage  = new IPTCProperties(this);
    d->page_properties = addPage(d->propertiesPage, i18nc("@item: iptc", "Properties"));
    d->page_properties->setIcon(QIcon::fromTheme(QLatin1String("draw-freehand")));

    d->envelopePage  = new IPTCEnvelope(this);
    d->page_envelope = addPage(d->envelopePage, i18nc("@item: iptc", "Envelope"));
    d->page_envelope->setIcon(QIcon::fromTheme(QLatin1String("mail-mark-unread")));

    // ---------------------------------------------------------------

    connect(d->contentPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->propertiesPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->subjectsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->keywordsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->categoriesPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->creditsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->statusPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->originPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->envelopePage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    // ------------------------------------------------------------

    readSettings();
    slotItemChanged();
}

IPTCEditWidget::~IPTCEditWidget()
{
    delete d;
}

void IPTCEditWidget::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("All Metadata Edit Settings"));
    showPage(group.readEntry(QLatin1String("All IPTC Edit Page"), 0));
    d->contentPage->setCheckedSyncJFIFComment(group.readEntry(QLatin1String("All Sync JFIF Comment"), true));
    d->contentPage->setCheckedSyncEXIFComment(group.readEntry(QLatin1String("All Sync EXIF Comment"), true));
    d->originPage->setCheckedSyncEXIFDate(group.readEntry(QLatin1String("All Sync EXIF Date"),        true));
}

void IPTCEditWidget::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("All Metadata Edit Settings"));
    group.writeEntry(QLatin1String("All IPTC Edit Page"),    activePageIndex());
    group.writeEntry(QLatin1String("All Sync JFIF Comment"), d->contentPage->syncJFIFCommentIsChecked());
    group.writeEntry(QLatin1String("All Sync EXIF Comment"), d->contentPage->syncEXIFCommentIsChecked());
    group.writeEntry(QLatin1String("All Sync EXIF Date"),    d->originPage->syncEXIFDateIsChecked());
    config->sync();
}

void IPTCEditWidget::slotItemChanged()
{
    d->page_content->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Content Information"))
                                       .arg(i18nc("@title", "Describe the visual content of the item"))));

    d->page_origin->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Origin Information"))
                                       .arg(i18nc("@title", "Formal descriptive information about the item"))));

    d->page_credits->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Credit Information"))
                                       .arg(i18nc("@title", "Record copyright information about the item"))));

    d->page_subjects->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Subject Information"))
                                       .arg(i18nc("@title", "Record subject information about the item"))));

    d->page_keywords->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Keyword Information"))
                                       .arg(i18nc("@title", "Record keywords relevant to the item"))));

    d->page_categories->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Category Information"))
                                       .arg(i18nc("@title", "Record categories relevant to the item"))));

    d->page_status->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Status Information"))
                                       .arg(i18nc("@title", "Record workflow information"))));

    d->page_properties->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Status Properties"))
                                       .arg(i18nc("@title", "Record workflow properties"))));

    d->page_envelope->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Envelope Information"))
                                       .arg(i18nc("@title", "Record editorial details"))));

    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->load((*d->dlg->currentItem()).toLocalFile());

    d->contentPage->readMetadata(*meta);
    d->originPage->readMetadata(*meta);
    d->creditsPage->readMetadata(*meta);
    d->subjectsPage->readMetadata(*meta);
    d->keywordsPage->readMetadata(*meta);
    d->categoriesPage->readMetadata(*meta);
    d->statusPage->readMetadata(*meta);
    d->propertiesPage->readMetadata(*meta);
    d->envelopePage->readMetadata(*meta);

    d->isReadOnly = (
                     (MetaEngineSettings::instance()->settings().metadataWritingMode == DMetadata::WRITE_TO_FILE_ONLY) &&
                     !QFileInfo((*d->dlg->currentItem()).toLocalFile()).isWritable()
                    );

    Q_EMIT signalSetReadOnly(d->isReadOnly);

    d->page_content->setEnabled(!d->isReadOnly);
    d->page_origin->setEnabled(!d->isReadOnly);
    d->page_credits->setEnabled(!d->isReadOnly);
    d->page_subjects->setEnabled(!d->isReadOnly);
    d->page_keywords->setEnabled(!d->isReadOnly);
    d->page_categories->setEnabled(!d->isReadOnly);
    d->page_status->setEnabled(!d->isReadOnly);
    d->page_properties->setEnabled(!d->isReadOnly);
    d->page_envelope->setEnabled(!d->isReadOnly);
}

void IPTCEditWidget::apply()
{
    if (d->modified && !d->isReadOnly)
    {
        QScopedPointer<DMetadata> meta(new DMetadata);
        meta->load((*d->dlg->currentItem()).toLocalFile());

        d->contentPage->applyMetadata(*meta);
        d->originPage->applyMetadata(*meta);
        d->creditsPage->applyMetadata(*meta);
        d->subjectsPage->applyMetadata(*meta);
        d->keywordsPage->applyMetadata(*meta);
        d->categoriesPage->applyMetadata(*meta);
        d->statusPage->applyMetadata(*meta);
        d->propertiesPage->applyMetadata(*meta);
        d->envelopePage->applyMetadata(*meta);

        meta->applyChanges();

        d->modified = false;
    }
}

void IPTCEditWidget::slotModified()
{
    if (!d->isReadOnly)
    {
        d->modified = true;

        Q_EMIT signalModified();
    }
}

void IPTCEditWidget::showPage(int page)
{
    switch (page)
    {
        case 0:
        {
            setCurrentPage(d->page_content);
            break;
        }

        case 1:
        {
            setCurrentPage(d->page_origin);
            break;
        }

        case 2:
        {
            setCurrentPage(d->page_credits);
            break;
        }

        case 3:
        {
            setCurrentPage(d->page_subjects);
            break;
        }

        case 4:
        {
            setCurrentPage(d->page_keywords);
            break;
        }

        case 5:
        {
            setCurrentPage(d->page_categories);
            break;
        }

        case 6:
        {
            setCurrentPage(d->page_status);
            break;
        }

        case 7:
        {
            setCurrentPage(d->page_properties);
            break;
        }

        case 8:
        {
            setCurrentPage(d->page_envelope);
            break;
        }

        default:
        {
            setCurrentPage(d->page_content);
            break;
        }
    }
}

int IPTCEditWidget::activePageIndex() const
{
    const DConfigDlgWdgItem* const cur = currentPage();

    if (cur == d->page_content)    return 0;
    if (cur == d->page_origin)     return 1;
    if (cur == d->page_credits)    return 2;
    if (cur == d->page_subjects)   return 3;
    if (cur == d->page_keywords)   return 4;
    if (cur == d->page_categories) return 5;
    if (cur == d->page_status)     return 6;
    if (cur == d->page_properties) return 7;
    if (cur == d->page_envelope)   return 8;

    return 0;
}

bool IPTCEditWidget::isModified() const
{
    return d->modified;
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_iptceditwidget.cpp"
