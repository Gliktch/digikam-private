/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008      by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "presentation_captionpage.h"

// Qt includes

#include <QFont>
#include <QFontDialog>
#include <QPalette>

// Local includes

#include "presentationcontainer.h"

namespace DigikamGenericPresentationPlugin
{

PresentationCaptionPage::PresentationCaptionPage(QWidget* const parent,
                                                 PresentationContainer* const sharedData)
    : QWidget(parent)
{
    setupUi(this);

    m_sharedData = sharedData;
    m_fontSampleLbl->setText(i18n("This is a comment sample..."));
    m_fontSampleLbl->setAutoFillBackground(true);
}

void PresentationCaptionPage::readSettings()
{
    connect(m_commentsFontColor, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotCommentsFontColorChanged()));

    connect(m_commentsBgColor, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotCommentsBgColorChanged()));

    connect(m_fontselectBtn, SIGNAL(clicked()),
            this, SLOT(slotOpenFontDialog()));

    m_commentsLinesLengthSpinBox->setValue(m_sharedData->commentsLinesLength);
    m_commentsFontColor->setColor(QColor(m_sharedData->commentsFontColor));
    m_commentsBgColor->setColor(QColor(m_sharedData->commentsBgColor));
    m_commentsDrawOutlineCheckBox->setChecked(m_sharedData->commentsDrawOutline);
    m_fontSampleLbl->setFont(*(m_sharedData->captionFont));
    m_commentsBgTransparency->setValue(m_sharedData->bgOpacity);

    slotCommentsBgColorChanged();
    slotCommentsFontColorChanged();
}

void PresentationCaptionPage::saveSettings()
{
    delete m_sharedData->captionFont;
    m_sharedData->captionFont         = new QFont(m_fontSampleLbl->font());
    QColor fontColor                  = QColor(m_commentsFontColor->color());
    m_sharedData->commentsFontColor   = fontColor.rgb();
    QColor bgColor                    = QColor(m_commentsBgColor->color());
    m_sharedData->commentsBgColor     = bgColor.rgb();
    m_sharedData->commentsDrawOutline = m_commentsDrawOutlineCheckBox->isChecked();
    m_sharedData->commentsLinesLength = m_commentsLinesLengthSpinBox->value();
    m_sharedData->bgOpacity           = m_commentsBgTransparency->value();
}

void PresentationCaptionPage::slotCommentsBgColorChanged()
{
    QPalette palette = m_fontSampleLbl->palette();
    palette.setColor(m_fontSampleLbl->backgroundRole(), m_commentsBgColor->color());
    m_fontSampleLbl->setPalette(palette);
}

void PresentationCaptionPage::slotCommentsFontColorChanged()
{
    QPalette palette = m_fontSampleLbl->palette();
    palette.setColor(m_fontSampleLbl->foregroundRole(), m_commentsFontColor->color());
    m_fontSampleLbl->setPalette(palette);
}

void PresentationCaptionPage::slotOpenFontDialog()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, *(m_sharedData->captionFont), this);

    if (ok)
    {
        m_fontSampleLbl->setFont(f);
    }
}

} // namespace DigikamGenericPresentationPlugin

#include "moc_presentation_captionpage.cpp"
