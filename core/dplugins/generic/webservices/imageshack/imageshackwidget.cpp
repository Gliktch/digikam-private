/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imageshackwidget_p.h"

namespace DigikamGenericImageShackPlugin
{

ImageShackWidget::ImageShackWidget(QWidget* const parent,
                                   ImageShackSession* const session,
                                   DInfoInterface* const iface,
                                   const QString& toolName)
    : WSSettingsWidget(parent, iface, toolName),
      d               (new Private)
{
    d->session            = session;
    d->iface              = iface;
    d->imgList            = imagesList();
    d->headerLbl          = getHeaderLbl();
    d->accountNameLbl     = getUserNameLabel();
    d->chgRegCodeBtn      = getChangeUserBtn();
    d->reloadGalleriesBtn = getReloadBtn();
    d->galleriesCob       = getAlbumsCoB();
    d->progressBar        = progressBar();

    connect(d->reloadGalleriesBtn, SIGNAL(clicked()),
            this, SLOT(slotReloadGalleries()));

    QGroupBox* const tagsBox      = new QGroupBox(QLatin1String(""), getSettingsBox());
    QGridLayout* const tagsLayout = new QGridLayout(tagsBox);

    d->privateImagesChb    = new QCheckBox(tagsBox);
    d->privateImagesChb->setText(i18nc("@option", "Make private"));
    d->privateImagesChb->setChecked(false);

    d->tagsFld             = new DTextEdit(tagsBox);
    d->tagsFld->setLinesVisible(1);
    QLabel* const tagsLbl  = new QLabel(i18nc("@label", "Tags (optional):"), tagsBox);

    d->remBarChb           = new QCheckBox(i18nc("@option", "Remove information bar on thumbnails"));
    d->remBarChb->setChecked(false);

    tagsLayout->addWidget(d->privateImagesChb, 0, 0);
    tagsLayout->addWidget(tagsLbl,             1, 0);
    tagsLayout->addWidget(d->tagsFld,          1, 1);

    addWidgetToSettingsBox(tagsBox);

    getUploadBox()->hide();
    getSizeBox()->hide();

    this->updateLabels();
}

ImageShackWidget::~ImageShackWidget()
{
    delete d;
}

void ImageShackWidget::updateLabels(const QString& /*name*/, const QString& /*url*/)
{
    if (d->session->loggedIn())
    {
        d->accountNameLbl->setText(d->session->username());
    }
    else
    {
        d->accountNameLbl->clear();
    }
}

void ImageShackWidget::slotGetGalleries(const QStringList& gTexts, const QStringList& gNames)
{
    d->galleriesCob->clear();

    d->galleriesCob->addItem(i18nc("@item:inlistbox", "Add to root folder"),
                             QLatin1String("--add-to-root--"));

    d->galleriesCob->addItem(i18nc("@item:inlistbox", "Create new gallery"),
                             QLatin1String("--new-gallery--"));

    // TODO check if the lists have the same size

    for (int i = 0 ; i < gTexts.size() ; ++i)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "gTexts is "<<gTexts[i] << " gNames is "<<gNames[i];
        d->galleriesCob->addItem(gTexts[i], gNames[i]);
    }

//     slotEnableNewGalleryLE(d->galleriesCob->currentIndex());
}

void ImageShackWidget::slotReloadGalleries()
{
    Q_EMIT signalReloadGalleries();
}

} // namespace DigikamGenericImageShackPlugin

#include "moc_imageshackwidget.cpp"
