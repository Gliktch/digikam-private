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

#include "advprintcaptionpage.h"

// Qt includes

#include <QIcon>
#include <QPrinterInfo>
#include <QWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "advprintwizard.h"
#include "advprintphoto.h"

namespace DigikamGenericPrintCreatorPlugin
{

class Q_DECL_HIDDEN AdvPrintCaptionPage::Private
{
public:

    template <class Ui_Class>

    class Q_DECL_HIDDEN WizardUI : public QWidget,
                                   public Ui_Class
    {
    public:

        explicit WizardUI(QWidget* const parent)
            : QWidget(parent)
        {
            this->setupUi(this);
        }
    };

    typedef WizardUI<Ui_AdvPrintCaptionPage> CaptionUI;

public:

    explicit Private(QWizard* const dialog)
    {
        captionUi = new CaptionUI(dialog);
        wizard    = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = wizard->iface();
        }
    }

public:

    CaptionUI*        captionUi = nullptr;
    AdvPrintWizard*   wizard    = nullptr;
    AdvPrintSettings* settings  = nullptr;
    DInfoInterface*   iface     = nullptr;
};

AdvPrintCaptionPage::AdvPrintCaptionPage(QWizard* const wizard, const QString& title)
    : DWizardPage(wizard, title),
      d          (new Private(wizard))
{
    QMap<AdvPrintSettings::CaptionType, QString> map                = AdvPrintSettings::captionTypeNames();
    QMap<AdvPrintSettings::CaptionType, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        d->captionUi->m_captionType->addItem(it.value(), (int)it.key());
        ++it;
    }

    d->captionUi->m_FreeCaptionFormat->setLinesVisible(1);

    // ----------------------------------------------------------------------

    connect(d->captionUi->m_captionType, SIGNAL(activated(int)),
            this, SLOT(slotCaptionChanged(int)));

    connect(d->captionUi->m_FreeCaptionFormat, SIGNAL(editingFinished()),
            this, SLOT(slotUpdateCaptions()));

    connect(d->captionUi->m_font_name, SIGNAL(currentFontChanged(QFont)),
            this, SLOT(slotUpdateCaptions()));

    connect(d->captionUi->m_font_size, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateCaptions()));

    connect(d->captionUi->m_font_color, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotUpdateCaptions()));

    connect(d->captionUi->mPrintList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotUpdateCaptions()));

    // -----------------------------------

    d->captionUi->mPrintList->setIface(d->iface);
    d->captionUi->mPrintList->setAllowDuplicate(true);
    d->captionUi->mPrintList->setControlButtonsPlacement(DItemsList::NoControlButtons);
    d->captionUi->mPrintList->listView()->setColumn(DItemsListView::User1,
                                                    i18nc("@title: column", "Caption"),
                                                    true);

    // -----------------------------------

    setPageWidget(d->captionUi);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("imagecomment")));
}

AdvPrintCaptionPage::~AdvPrintCaptionPage()
{
    delete d;
}

DItemsList* AdvPrintCaptionPage::imagesList() const
{
    return d->captionUi->mPrintList;
}

void AdvPrintCaptionPage::initializePage()
{
    d->captionUi->m_captionType->setCurrentIndex(d->settings->captionType);
    enableCaptionGroup(d->captionUi->m_captionType->currentIndex());
    d->captionUi->m_font_color->setColor(d->settings->captionColor);
    d->captionUi->m_font_name->setCurrentFont(d->settings->captionFont.family());
    d->captionUi->m_font_size->setValue(d->settings->captionSize);
    d->captionUi->m_FreeCaptionFormat->setText(d->settings->captionTxt);
    slotCaptionChanged(d->captionUi->m_captionType->currentIndex());
    slotUpdateImagesList();
    slotUpdateCaptions();
}

bool AdvPrintCaptionPage::validatePage()
{
    d->settings->captionType  = (AdvPrintSettings::CaptionType)d->captionUi->m_captionType->currentIndex();
    d->settings->captionColor = d->captionUi->m_font_color->color();
    d->settings->captionFont  = QFont(d->captionUi->m_font_name->currentFont());
    d->settings->captionSize  = d->captionUi->m_font_size->value();
    d->settings->captionTxt   = d->captionUi->m_FreeCaptionFormat->text();

    return true;
}

void AdvPrintCaptionPage::slotUpdateImagesList()
{
    d->captionUi->mPrintList->listView()->clear();
    d->captionUi->mPrintList->slotAddImages(d->wizard->itemsList());
}

void AdvPrintCaptionPage::blockCaptionButtons(bool block)
{
    d->captionUi->m_captionType->blockSignals(block);
    d->captionUi->m_free_label->blockSignals(block);
    d->captionUi->m_font_name->blockSignals(block);
    d->captionUi->m_font_size->blockSignals(block);
    d->captionUi->m_font_color->blockSignals(block);
}

void AdvPrintCaptionPage::enableCaptionGroup(int index)
{
    bool fontSettingsEnabled;

    if      (index == AdvPrintSettings::NONE)
    {
        fontSettingsEnabled = false;
        d->captionUi->m_customCaptionGB->setEnabled(false);
    }
    else if (index == AdvPrintSettings::CUSTOM)
    {
        fontSettingsEnabled = true;
        d->captionUi->m_customCaptionGB->setEnabled(true);
    }
    else
    {
        fontSettingsEnabled = true;
        d->captionUi->m_customCaptionGB->setEnabled(false);
    }

    d->captionUi->m_font_name->setEnabled(fontSettingsEnabled);
    d->captionUi->m_font_size->setEnabled(fontSettingsEnabled);
    d->captionUi->m_font_color->setEnabled(fontSettingsEnabled);
}

void AdvPrintCaptionPage::slotCaptionChanged(int index)
{
    enableCaptionGroup(index);
    slotUpdateCaptions();
}

void AdvPrintCaptionPage::updateCaption(AdvPrintPhoto* const pPhoto)
{
    if (pPhoto)
    {
        if      (!pPhoto->m_pAdvPrintCaptionInfo &&
                 (d->captionUi->m_captionType->currentIndex() != AdvPrintSettings::NONE))
        {
            pPhoto->m_pAdvPrintCaptionInfo = new AdvPrintCaptionInfo();
        }
        else if (pPhoto->m_pAdvPrintCaptionInfo &&
                 (d->captionUi->m_captionType->currentIndex() == AdvPrintSettings::NONE))
        {
            delete pPhoto->m_pAdvPrintCaptionInfo;
            pPhoto->m_pAdvPrintCaptionInfo = nullptr;
        }

        if (pPhoto->m_pAdvPrintCaptionInfo)
        {
            pPhoto->m_pAdvPrintCaptionInfo->m_captionColor =
                d->captionUi->m_font_color->color();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionSize  =
                d->captionUi->m_font_size->value();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionFont  =
                d->captionUi->m_font_name->currentFont();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionType  =
                (AdvPrintSettings::CaptionType)d->captionUi->m_captionType->currentIndex();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionText  =
                d->captionUi->m_FreeCaptionFormat->text();

            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Update caption properties for"
                                                 << pPhoto->m_url;
        }
    }
}

void AdvPrintCaptionPage::slotUpdateCaptions()
{
    if (d->settings->photos.size())
    {
        for (AdvPrintPhoto* const pPhoto : std::as_const(d->settings->photos))
        {
            updateCaption(pPhoto);

            if (pPhoto && pPhoto->m_pAdvPrintCaptionInfo)
            {
                DItemsListViewItem* const lvItem = d->captionUi->mPrintList->listView()->findItem(pPhoto->m_url);

                if (lvItem)
                {
                    QString cap;

                    if (pPhoto->m_pAdvPrintCaptionInfo->m_captionType != AdvPrintSettings::NONE)
                    {
                        cap = pPhoto->formattedCaption();
                    }

                    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << cap;

                    lvItem->setText(DItemsListView::User1, cap);
                }
            }
        }
    }

    // create our photo sizes list

    d->wizard->previewPhotos();
}

} // namespace DigikamGenericPrintCreatorPlugin

#include "moc_advprintcaptionpage.cpp"
