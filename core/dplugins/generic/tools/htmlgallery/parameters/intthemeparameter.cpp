/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "intthemeparameter.h"

// Qt includes

#include <QSpinBox>

// KDE includes

#include <kconfiggroup.h>

static const char* MIN_VALUE_KEY = "Min";
static const char* MAX_VALUE_KEY = "Max";

namespace DigikamGenericHtmlGalleryPlugin
{

class Q_DECL_HIDDEN IntThemeParameter::Private
{
public:

    Private() = default;

    int minValue = 0;
    int maxValue = 99999;
};

IntThemeParameter::IntThemeParameter()
    : d(new Private)
{
}

IntThemeParameter::~IntThemeParameter()
{
    delete d;
}

void IntThemeParameter::init(const QByteArray& internalName, const KConfigGroup* configGroup)
{
    AbstractThemeParameter::init(internalName, configGroup);

    d->minValue = configGroup->readEntry(MIN_VALUE_KEY, 0);
    d->maxValue = configGroup->readEntry(MAX_VALUE_KEY, 99999);
}

QWidget* IntThemeParameter::createWidget(QWidget* parent, const QString& value) const
{
    QSpinBox* const spinBox = new QSpinBox(parent);
    spinBox->setValue(value.toInt());
    spinBox->setMinimum(d->minValue);
    spinBox->setMaximum(d->maxValue);

    return spinBox;
}

QString IntThemeParameter::valueFromWidget(QWidget* widget) const
{
    QSpinBox* const spinBox = static_cast<QSpinBox*>(widget);

    return QString::number(spinBox->value());
}

} // namespace DigikamGenericHtmlGalleryPlugin
