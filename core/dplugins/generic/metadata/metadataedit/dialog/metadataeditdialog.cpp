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

#include "metadataeditdialog.h"

// Qt includes

#include <QKeyEvent>
#include <QPointer>
#include <QObject>
#include <QApplication>
#include <QMenu>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QImage>
#include <QBuffer>
#include <QPainter>
#include <QPalette>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "exifeditwidget.h"
#include "iptceditwidget.h"
#include "xmpeditwidget.h"
#include "thumbnailloadthread.h"
#include "dxmlguiwindow.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN MetadataEditDialog::Private
{
public:

    Private() = default;

public:

    bool                   isReadOnly   = false;

    QString                preview;

    QList<QUrl>            urls;
    QList<QUrl>::iterator  currItem;

    QTabWidget*            tabWidget    = nullptr;

    EXIFEditWidget*        tabExif      = nullptr;
    IPTCEditWidget*        tabIptc      = nullptr;
    XMPEditWidget*         tabXmp       = nullptr;

    ThumbnailImageCatcher* catcher      = nullptr;

    DInfoInterface*        iface        = nullptr;
};

MetadataEditDialog::MetadataEditDialog(QWidget* const parent, DInfoInterface* const iface)
    : DPluginDialog(parent, QLatin1String("Metadata Edit Dialog")),
      d            (new Private)
{
    setWindowTitle(i18nc("@title:window", "Metadata Editor"));
    setModal(true);

    d->iface                          = iface;

    ThumbnailLoadThread* const thread = new ThumbnailLoadThread;
    thread->setThumbnailSize(48);
    thread->setPixmapRequested(false);
    d->catcher                        = new ThumbnailImageCatcher(thread, this);

    d->urls     = d->iface->currentSelectedItems();
    d->currItem = d->urls.begin();

    if (!d->iface->currentActiveItem().isEmpty())
    {
        QList<QUrl>::iterator it = d->currItem;

        for ( ; it != d->urls.end() ; ++it)
        {
            if (*it == d->iface->currentActiveItem())
            {
                d->currItem = it;
                break;
            }
        }
    }

    updatePreview();

    QDialogButtonBox::StandardButtons btns = QDialogButtonBox::Ok    |
                                             QDialogButtonBox::Apply |
                                             QDialogButtonBox::Close |
                                             QDialogButtonBox::No    |   // NextPrevious item
                                             QDialogButtonBox::Yes;      // Previous item

    m_buttons = new QDialogButtonBox(btns, this);
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    m_buttons->button(QDialogButtonBox::Apply)->setEnabled(false);
    m_buttons->button(QDialogButtonBox::No)->setText(i18nc("@action: button",  "Next"));
    m_buttons->button(QDialogButtonBox::No)->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
    m_buttons->button(QDialogButtonBox::Yes)->setText(i18nc("@action: button", "Previous"));
    m_buttons->button(QDialogButtonBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));

    if (d->urls.count() <= 1)
    {
        m_buttons->button(QDialogButtonBox::No)->setDisabled(true);
        m_buttons->button(QDialogButtonBox::Yes)->setDisabled(true);
    }

    d->tabWidget = new QTabWidget(this);
    d->tabExif   = new EXIFEditWidget(this);
    d->tabIptc   = new IPTCEditWidget(this);
    d->tabXmp    = new XMPEditWidget(this);
    d->tabWidget->addTab(d->tabExif, i18nc("@item", "Edit EXIF"));
    d->tabWidget->addTab(d->tabIptc, i18nc("@item", "Edit IPTC"));
    d->tabWidget->addTab(d->tabXmp,  i18nc("@item", "Edit XMP"));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->tabWidget);
    vbx->addWidget(m_buttons);
    setLayout(vbx);

    //----------------------------------------------------------

    connect(d->tabExif, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->tabIptc, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->tabXmp, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->tabExif, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    connect(d->tabIptc, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    connect(d->tabXmp, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    connect(m_buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(slotApply()));

    connect(m_buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOk()));

    connect(m_buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(slotClose()));

    connect(m_buttons->button(QDialogButtonBox::No), SIGNAL(clicked()),
            this, SLOT(slotNext()));

    connect(m_buttons->button(QDialogButtonBox::Yes), SIGNAL(clicked()),
            this, SLOT(slotPrevious()));

    connect(this, SIGNAL(signalMetadataChangedForUrl(QUrl)),
            d->iface, SLOT(slotMetadataChangedForUrl(QUrl)));

    //----------------------------------------------------------

    readSettings();
    slotItemChanged();
}

MetadataEditDialog::~MetadataEditDialog()
{
    d->catcher->thread()->stopAllTasks();
    d->catcher->cancel();

    delete d->catcher->thread();
    delete d->catcher;
    delete d;
}

QString MetadataEditDialog::currentItemTitleHeader(const QString& title) const
{
    QString start = QLatin1String("<qt><table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">");
    QString end   = QLatin1String("</table></qt>");

    return QString::fromLatin1("%1<tr><td>%2</td><td>%3</td></tr>%4").arg(start).arg(d->preview).arg(title).arg(end);
}

void MetadataEditDialog::updatePreview()
{
    d->catcher->setActive(true);

    d->catcher->thread()->find(ThumbnailIdentifier(d->currItem->toLocalFile()));
    d->catcher->enqueue();
    QList<QImage> images = d->catcher->waitForThumbnails();

    QImage img(48, 48, QImage::Format_ARGB32);
    QImage thm           = images.first();
    QPainter p(&img);
    p.fillRect(img.rect(), QPalette().window());
    p.setPen(Qt::black);
    p.drawRect(img.rect().left(), img.rect().top(), img.rect().right()-1, img.rect().bottom()-1);
    p.drawImage((img.width() - thm.width())/2, (img.height() - thm.height())/2, thm);

    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");
    d->preview = QString::fromLatin1("<img src=\"data:image/png;base64,%1\">  ").arg(QLatin1String(byteArray.toBase64().data()));

    d->catcher->setActive(false);
}

QList<QUrl>::iterator MetadataEditDialog::currentItem() const
{
    return d->currItem;
}

void MetadataEditDialog::slotModified()
{
    bool modified = false;

    switch (d->tabWidget->currentIndex())
    {
        case 0:
        {
            modified = d->tabExif->isModified();
            break;
        }

        case 1:
        {
            modified = d->tabIptc->isModified();
            break;
        }

        case 2:
        {
            modified = d->tabXmp->isModified();
            break;
        }
    }

    m_buttons->button(QDialogButtonBox::Apply)->setEnabled(modified);
}

void MetadataEditDialog::slotOk()
{
    slotApply();
    saveSettings();
    accept();
}

void MetadataEditDialog::slotClose()
{
    saveSettings();
    close();
}

void MetadataEditDialog::slotApply()
{
    d->tabExif->apply();
    d->tabIptc->apply();
    d->tabXmp->apply();

    Q_EMIT signalMetadataChangedForUrl(*d->currItem);

    slotItemChanged();
}

void MetadataEditDialog::slotNext()
{
    slotApply();
    ++d->currItem;
    slotItemChanged();
}

void MetadataEditDialog::slotPrevious()
{
    slotApply();
    --d->currItem;
    slotItemChanged();
}

void MetadataEditDialog::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Metadata Edit Dialog"));
    d->tabWidget->setCurrentIndex(group.readEntry(QLatin1String("Tab Index"), 0));
}

void MetadataEditDialog::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Metadata Edit Dialog"));
    group.writeEntry(QLatin1String("Tab Index"), d->tabWidget->currentIndex());

    d->tabExif->saveSettings();
    d->tabIptc->saveSettings();
    d->tabXmp->saveSettings();
}

void MetadataEditDialog::slotItemChanged()
{
    updatePreview();
    d->tabExif->slotItemChanged();
    d->tabIptc->slotItemChanged();
    d->tabXmp->slotItemChanged();

    setWindowTitle(i18nc("@title:window", "%1 (%2/%3) - Edit Metadata",
                   (*d->currItem).fileName(),
                   d->urls.indexOf(*(d->currItem))+1,
                   d->urls.count()));

    m_buttons->button(QDialogButtonBox::No)->setEnabled(*(d->currItem) != d->urls.last());
    m_buttons->button(QDialogButtonBox::Yes)->setEnabled(*(d->currItem) != d->urls.first());
    m_buttons->button(QDialogButtonBox::Apply)->setEnabled(!d->isReadOnly);
}

bool MetadataEditDialog::eventFilter(QObject*, QEvent* e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* const k = dynamic_cast<QKeyEvent*>(e);

        if      (
                 k &&
                 (
                  (k->modifiers() == Qt::ControlModifier) &&
                  ((k->key() == Qt::Key_Enter) || (k->key() == Qt::Key_Return))
                 )
                )
        {
            slotApply();

            if (m_buttons->button(QDialogButtonBox::No)->isEnabled())
            {
                slotNext();
            }

            return true;
        }
        else if (
                 k &&
                 (
                  (k->modifiers() == Qt::ShiftModifier) &&
                  ((k->key() == Qt::Key_Enter) || (k->key() == Qt::Key_Return))
                 )
                )
        {
            slotApply();

            if (m_buttons->button(QDialogButtonBox::Yes)->isEnabled())
            {
                slotPrevious();
            }

            return true;
        }

        return false;
    }

    return false;
}

void MetadataEditDialog::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    saveSettings();
    e->accept();
}

void MetadataEditDialog::slotSetReadOnly(bool state)
{
    d->isReadOnly = state;
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_metadataeditdialog.cpp"
