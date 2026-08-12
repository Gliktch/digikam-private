/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: File properties
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tableview_column_file.h"

// Qt includes

#include <QFormLayout>
#include <QComboBox>
#include <QLocale>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "itempropertiestab.h"
#include "iteminfo.h"

namespace Digikam
{

namespace TableViewColumns
{

ColumnFileProperties::ColumnFileProperties(TableViewShared* const tableViewShared,
                                           const TableViewColumnConfiguration& pConfiguration,
                                           const SubColumn pSubColumn,
                                           QObject* const parent)
    : TableViewColumn(tableViewShared, pConfiguration, parent),
      subColumn      (pSubColumn)
{
}

TableViewColumnDescription ColumnFileProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("file-properties"), i18n("File properties"));
    description.setIcon(QLatin1String("dialog-information"));

    description.addSubColumn(TableViewColumnDescription(QLatin1String("filename"),         i18n("Filename")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("filepath"),         i18n("Path")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("filesize"),         i18n("Size")));
    description.addSubColumn(TableViewColumnDescription(QLatin1String("filelastmodified"), i18n("Last modified")));

    return description;
}

QStringList ColumnFileProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("filename")
            << QLatin1String("filepath")
            << QLatin1String("filesize")
            << QLatin1String("filelastmodified");

    return columns;
}

QString ColumnFileProperties::getTitle() const
{
    switch (subColumn)
    {
        case SubColumnName:
        {
            return i18n("Filename");
        }

        case SubColumnFilePath:
        {
            return i18n("Path");
        }

        case SubColumnSize:
        {
            return i18n("Size");
        }

        case SubColumnLastModified:
        {
            return i18n("Last modified");
        }
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnFileProperties::getColumnFlags() const
{
    if (
        (subColumn == SubColumnSize) ||
        (subColumn == SubColumnLastModified)
       )
    {
        return (ColumnCustomSorting | ColumnHasConfigurationWidget);
    }

    return ColumnNoFlags;
}

QVariant ColumnFileProperties::data(TableViewModel::Item* const item, const int role) const
{
    if (
        (role != Qt::DisplayRole) &&
        (role != Qt::TextAlignmentRole)
       )
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        switch (subColumn)
        {
            case SubColumnSize:
            {
                return QVariant(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter));
            }

            default:
            {
                return QVariant();
            }
        }
    }

    const ItemInfo info = s->tableViewModel->infoFromItem(item);

    switch (subColumn)
    {
        case SubColumnName:
        {
            return info.fileUrl().fileName();
        }

        case SubColumnFilePath:
        {
            return QDir::toNativeSeparators(info.fileUrl().toLocalFile());
        }

        case SubColumnSize:
        {
            /// @todo Add configuration options for SI-prefixes
            /// @todo Use an enum instead to avoid lots of string comparisons

            const QString formatKey = configuration.getSetting(QLatin1String("format"), QLatin1String("human"));

            if (formatKey == QLatin1String("human"))
            {
                return ItemPropertiesTab::humanReadableBytesCount(info.fileSize());
            }
            else
            {
                // formatKey == "plain"

                return QLocale().toString(info.fileSize());
            }

            break;
        }

        case SubColumnLastModified:
        {
            return asUserDateTime(info.modDateTime());
        }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnFileProperties::compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const
{
    const ItemInfo infoA = s->tableViewModel->infoFromItem(itemA);
    const ItemInfo infoB = s->tableViewModel->infoFromItem(itemB);

    switch (subColumn)
    {
        case SubColumnSize:
        {
            const int sizeA = infoA.fileSize();
            const int sizeB = infoB.fileSize();

            return compareHelper<int>(sizeA, sizeB);
        }

        case SubColumnLastModified:
        {
            const QDateTime dtA = infoA.modDateTime();
            const QDateTime dtB = infoB.modDateTime();

            return compareHelper<QDateTime>(dtA, dtB);
        }

        default:
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "file: unimplemented comparison, subColumn=" << subColumn;

            return CmpEqual;
        }
    }
}

// ---------------------------------------------------------------------------------------

ColumnFileConfigurationWidget::ColumnFileConfigurationWidget(TableViewShared* const sharedObject,
                                                             const TableViewColumnConfiguration& columnConfiguration,
                                                             QWidget* const parentWidget)
    : TableViewColumnConfigurationWidget(sharedObject, columnConfiguration, parentWidget)
{
    ColumnFileProperties::getSubColumnIndex<ColumnFileProperties>(configuration.columnId, &subColumn);

    switch (subColumn)
    {
        case ColumnFileProperties::SubColumnSize:
        {
            QFormLayout* const box1 = new QFormLayout();
            selectorSizeType        = new QComboBox(this);
            selectorSizeType->addItem(i18nc("@item: tableview", "Human readable"), QLatin1String("human"));
            selectorSizeType->addItem(i18nc("@item: tableview", "Plain"),          QLatin1String("plain"));
            box1->addRow(i18n("Display format"), selectorSizeType);

            setLayout(box1);

            const int index = selectorSizeType->findData(configuration.getSetting(QLatin1String("format"), QLatin1String("human")));
            selectorSizeType->setCurrentIndex((index >= 0) ? index : 0);
            break;
        }

        default:
        {
            break;
        }
    }
}

TableViewColumnConfiguration ColumnFileConfigurationWidget::getNewConfiguration()
{
    const QString formatKey = selectorSizeType->itemData(selectorSizeType->currentIndex()).toString();
    configuration.columnSettings.insert(QLatin1String("format"), formatKey);

    return configuration;
}

void ColumnFileProperties::setConfiguration(const TableViewColumnConfiguration& newConfiguration)
{
    configuration = newConfiguration;

    Q_EMIT signalAllDataChanged();
}

TableViewColumnConfigurationWidget* ColumnFileProperties::getConfigurationWidget(QWidget* const parentWidget) const
{
    return new ColumnFileConfigurationWidget(s, configuration, parentWidget);
}

} // namespace TableViewColumns

} // namespace Digikam


#include "moc_tableview_column_file.cpp"
