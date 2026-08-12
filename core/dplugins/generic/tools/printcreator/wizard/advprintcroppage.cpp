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

#include "advprintcroppage.h"

// Qt includes

#include <QIcon>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "advprintwizard.h"
#include "advprintphoto.h"

namespace DigikamGenericPrintCreatorPlugin
{

class Q_DECL_HIDDEN AdvPrintCropPage::Private
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

    typedef WizardUI<Ui_AdvPrintCropPage> CropUI;

public:

    explicit Private(QWizard* const dialog)
    {
        cropUi = new CropUI(dialog);
        wizard = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = wizard->iface();
        }
    }

    CropUI*           cropUi    = nullptr;
    AdvPrintWizard*   wizard    = nullptr;
    AdvPrintSettings* settings  = nullptr;
    DInfoInterface*   iface     = nullptr;
};

AdvPrintCropPage::AdvPrintCropPage(QWizard* const wizard, const QString& title)
    : DWizardPage(wizard, title),
      d          (new Private(wizard))
{
    d->cropUi->BtnCropRotateRight->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-right"))
                                           .pixmap(16, 16));
    d->cropUi->BtnCropRotateLeft->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-left"))
                                          .pixmap(16, 16));

    // -----------------------------------

    connect(d->cropUi->m_disableCrop, SIGNAL(stateChanged(int)),
            this, SLOT(slotCropSelection(int)));

    connect(d->cropUi->BtnCropPrev, SIGNAL(clicked()),
            this, SLOT(slotBtnCropPrevClicked()));

    connect(d->cropUi->BtnCropNext, SIGNAL(clicked()),
            this, SLOT(slotBtnCropNextClicked()));

    connect(d->cropUi->BtnCropRotateRight, SIGNAL(clicked()),
            this, SLOT(slotBtnCropRotateRightClicked()));

    connect(d->cropUi->BtnCropRotateLeft, SIGNAL(clicked()),
            this, SLOT(slotBtnCropRotateLeftClicked()));

    // -----------------------------------

    setPageWidget(d->cropUi);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("transform-crop")));
}

AdvPrintCropPage::~AdvPrintCropPage()
{
    delete d;
}

Ui_AdvPrintCropPage* AdvPrintCropPage::ui() const
{
    return d->cropUi;
}

void AdvPrintCropPage::slotCropSelection(int)
{
    d->cropUi->cropFrame->drawCropRectangle(!d->cropUi->m_disableCrop->isChecked());
    d->cropUi->update();
}

void AdvPrintCropPage::initializePage()
{
    d->settings->currentCropPhoto = 0;

    if (d->settings->photos.size())
    {
        AdvPrintPhoto* const photo = d->settings->photos[d->settings->currentCropPhoto];
        setBtnCropEnabled();
        d->cropUi->update();
        d->wizard->updateCropFrame(photo, d->settings->currentCropPhoto);
    }
}

bool AdvPrintCropPage::validatePage()
{
    d->settings->disableCrop = d->cropUi->m_disableCrop->isChecked();

    return true;
}

void AdvPrintCropPage::slotBtnCropPrevClicked()
{
    AdvPrintPhoto* const photo = d->settings->photos[--d->settings->currentCropPhoto];

    setBtnCropEnabled();

    if (!photo)
    {
        d->settings->currentCropPhoto = 0;
        return;
    }

    d->wizard->updateCropFrame(photo, d->settings->currentCropPhoto);
}

void AdvPrintCropPage::slotBtnCropNextClicked()
{
    AdvPrintPhoto* const photo = d->settings->photos[++d->settings->currentCropPhoto];
    setBtnCropEnabled();

    if (!photo)
    {
        d->settings->currentCropPhoto = d->settings->photos.count() - 1;
        return;
    }

    d->wizard->updateCropFrame(photo, d->settings->currentCropPhoto);
}

void AdvPrintCropPage::setBtnCropEnabled()
{
    if (d->settings->currentCropPhoto == 0)
    {
        d->cropUi->BtnCropPrev->setEnabled(false);
    }
    else
    {
        d->cropUi->BtnCropPrev->setEnabled(true);
    }

    if (d->settings->currentCropPhoto == (int)d->settings->photos.count() - 1)
    {
        d->cropUi->BtnCropNext->setEnabled(false);
    }
    else
    {
        d->cropUi->BtnCropNext->setEnabled(true);
    }
}

void AdvPrintCropPage::slotBtnCropRotateLeftClicked()
{
    // by definition, the cropRegion should be set by now,
    // which means that after our rotation it will become invalid,
    // so we will initialize it to -2 in an awful hack (this
    // tells the cropFrame to reset the crop region, but don't
    // automatically rotate the image to fit.

    AdvPrintPhoto* const photo = d->settings->photos[d->settings->currentCropPhoto];
    photo->m_cropRegion        = QRect(-2, -2, -2, -2);
    photo->m_rotation          = (photo->m_rotation - 90) % 360;

    d->wizard->updateCropFrame(photo, d->settings->currentCropPhoto);
}

void AdvPrintCropPage::slotBtnCropRotateRightClicked()
{
    // by definition, the cropRegion should be set by now,
    // which means that after our rotation it will become invalid,
    // so we will initialize it to -2 in an awful hack (this
    // tells the cropFrame to reset the crop region, but don't
    // automatically rotate the image to fit.

    AdvPrintPhoto* const photo = d->settings->photos[d->settings->currentCropPhoto];
    photo->m_cropRegion        = QRect(-2, -2, -2, -2);
    photo->m_rotation          = (photo->m_rotation + 90) % 360;

    d->wizard->updateCropFrame(photo, d->settings->currentCropPhoto);
}

} // namespace DigikamGenericPrintCreatorPlugin

#include "moc_advprintcroppage.cpp"
