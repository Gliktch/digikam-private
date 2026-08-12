/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : Texture settings view.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "texturesettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "dcombobox.h"
#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN TextureSettings::Private
{
public:

    Private() = default;

    const QString configTextureTypeEntry    = QLatin1String("TextureType");
    const QString configBlendGainEntry      = QLatin1String("BlendGain");

    DComboBox*           textureType        = nullptr;
    DIntNumInput*        blendGain          = nullptr;
};

// --------------------------------------------------------

TextureSettings::TextureSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing       = layoutSpacing();


    QGridLayout* const grid = new QGridLayout(parent);

    // -------------------------------------------------------------

    QLabel* const label1 = new QLabel(i18nc("@label", "Type:"));
    d->textureType       = new DComboBox;
    d->textureType->addItem(i18nc("@item: texture type", "Paper"));
    d->textureType->addItem(i18nc("@item: texture type", "Paper 2"));
    d->textureType->addItem(i18nc("@item: texture type", "Fabric"));
    d->textureType->addItem(i18nc("@item: texture type", "Burlap"));
    d->textureType->addItem(i18nc("@item: texture type", "Bricks"));
    d->textureType->addItem(i18nc("@item: texture type", "Bricks 2"));
    d->textureType->addItem(i18nc("@item: texture type", "Canvas"));
    d->textureType->addItem(i18nc("@item: texture type", "Marble"));
    d->textureType->addItem(i18nc("@item: texture type", "Marble 2"));
    d->textureType->addItem(i18nc("@item: texture type", "Blue Jean"));
    d->textureType->addItem(i18nc("@item: texture type", "Cell Wood"));
    d->textureType->addItem(i18nc("@item: texture type", "Metal Wire"));
    d->textureType->addItem(i18nc("@item: texture type", "Modern"));
    d->textureType->addItem(i18nc("@item: texture type", "Wall"));
    d->textureType->addItem(i18nc("@item: texture type", "Moss"));
    d->textureType->addItem(i18nc("@item: texture type", "Stone"));
    d->textureType->setDefaultIndex(TextureContainer::PaperTexture);
    d->textureType->setWhatsThis(i18nc("@info", "Set here the texture type to apply to image."));

    // -------------------------------------------------------------

    QLabel* const label2 = new QLabel(i18nc("@label", "Relief:"));
    d->blendGain         = new DIntNumInput;
    d->blendGain->setRange(1, 255, 1);
    d->blendGain->setDefaultValue(200);
    d->blendGain->setWhatsThis(i18nc("@info", "Set here the relief gain used to merge texture and image."));

    // -------------------------------------------------------------------

    grid->addWidget(label1,         0, 0, 1, 1);
    grid->addWidget(d->textureType, 0, 1, 1, 1);
    grid->addWidget(label2,         1, 0, 1, 2);
    grid->addWidget(d->blendGain,   2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->textureType, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->blendGain, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

TextureSettings::~TextureSettings()
{
    delete d;
}

TextureContainer TextureSettings::settings() const
{
    TextureContainer prm;

    prm.textureType = d->textureType->currentIndex();
    prm.blendGain   = 255 - d->blendGain->value();

    return prm;
}

void TextureSettings::setSettings(const TextureContainer& settings)
{
    blockSignals(true);

    d->textureType->setCurrentIndex(settings.textureType);
    d->blendGain->setValue(255 + settings.blendGain);

    blockSignals(false);
}

void TextureSettings::resetToDefault()
{
    setSettings(defaultSettings());
}

TextureContainer TextureSettings::defaultSettings() const
{
    TextureContainer prm;
    return prm;
}

void TextureSettings::readSettings(const KConfigGroup& group)
{
    blockSignals(true);

    d->textureType->setCurrentIndex(group.readEntry(d->configTextureTypeEntry, d->textureType->defaultIndex()));
    d->blendGain->setValue(group.readEntry(d->configBlendGainEntry,            d->blendGain->defaultValue()));

    blockSignals(false);
}

void TextureSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configTextureTypeEntry, d->textureType->currentIndex());
    group.writeEntry(d->configBlendGainEntry,   d->blendGain->value());
}

} // namespace Digikam

#include "moc_texturesettings.cpp"
