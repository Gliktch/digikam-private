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

#include "listthemeparameter.h"

// Qt includes

#include <QComboBox>

// KDE includes

#include <kconfiggroup.h>

namespace DigikamGenericHtmlGalleryPlugin
{

static const char* ITEM_VALUE_KEY   = "Value-";
static const char* ITEM_CAPTION_KEY = "Caption-";

class Q_DECL_HIDDEN ListThemeParameter::Private
{
public:

    Private() = default;

    QStringList            mOrderedValueList;
    QMap<QString, QString> mContentMap;
};

ListThemeParameter::ListThemeParameter()
    : d(new Private)
{
}

ListThemeParameter::~ListThemeParameter()
{
    delete d;
}

void ListThemeParameter::init(const QByteArray& internalName, const KConfigGroup* configGroup)
{
    AbstractThemeParameter::init(internalName, configGroup);

    for (int pos = 0 ; ; ++pos)
    {
        QString valueKey   = QString::fromUtf8("%1%2").arg(QLatin1String(ITEM_VALUE_KEY)).arg(pos);
        QString captionKey = QString::fromUtf8("%1%2").arg(QLatin1String(ITEM_CAPTION_KEY)).arg(pos);

        if (!configGroup->hasKey(valueKey) || !configGroup->hasKey(captionKey))
        {
            break;
        }

        QString value         = configGroup->readEntry(valueKey);
        QString caption       = configGroup->readEntry(captionKey);

        d->mOrderedValueList << value;
        d->mContentMap[value] = caption;
    }
}

QWidget* ListThemeParameter::createWidget(QWidget* parent, const QString& widgetDefaultValue) const
{
    QComboBox* const comboBox      = new QComboBox(parent);
    QStringList::ConstIterator it  = d->mOrderedValueList.constBegin();
    QStringList::ConstIterator end = d->mOrderedValueList.constEnd();

    for (; it != end ; ++it)
    {
        QString value   = *it;
        QString caption = d->mContentMap[value];
        comboBox->addItem(caption);

        if (value == widgetDefaultValue)
        {
            comboBox->setCurrentIndex(comboBox->count() - 1);
        }
    }

    return comboBox;
}

QString ListThemeParameter::valueFromWidget(QWidget* widget) const
{
    Q_ASSERT(widget);

    QComboBox* const comboBox = static_cast<QComboBox*>(widget);

    return d->mOrderedValueList[comboBox->currentIndex()];
}

} // namespace DigikamGenericHtmlGalleryPlugin
