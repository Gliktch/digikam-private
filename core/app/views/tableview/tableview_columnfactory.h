/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Table view column helpers
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QHash>
#include <QWidget>

// Local includes

#include "tableview_model.h"
#include "tableview_shared.h"

class QModelIndex;
class QStyleOptionViewItem;

class KConfigGroup;

namespace Digikam
{

class ItemInfo;
class ThumbnailSize;

class TableViewColumnConfiguration
{
public:

    explicit TableViewColumnConfiguration(const QString& id = QString())
      : columnId(id),
        columnSettings()
    {
    }

    QString getSetting(const QString& key, const QString& defaultValue = QString()) const
    {
        if (!columnSettings.contains(key))
        {
            return defaultValue;
        }

        return columnSettings.value(key);
    }

    void loadSettings(const KConfigGroup& configGroup);
    void saveSettings(KConfigGroup& configGroup) const;

public:

    QString                 columnId;
    QHash<QString, QString> columnSettings;
};

// ----------------------------------------------------------------------------

class TableViewColumnDescription
{
public:

    explicit TableViewColumnDescription()
      : columnId(),
        columnTitle(),
        columnIcon(),
        columnSettings(),
        subColumns()
    {
    }

    explicit TableViewColumnDescription(const QString& id, const QString& title,
                                        const QString& settingKey = QString(),
                                        const QString& settingValue = QString())
      : columnId(id),
        columnTitle(title),
        columnIcon(),
        columnSettings(),
        subColumns()
    {
        if (!settingKey.isEmpty())
        {
            addSetting(settingKey, settingValue);
        }
    }

public:

    typedef QList<TableViewColumnDescription> List;

    QString                                   columnId;
    QString                                   columnTitle;
    QString                                   columnIcon;
    QHash<QString, QString>                   columnSettings;
    QList<TableViewColumnDescription>         subColumns;

public:

    void addSubColumn(const TableViewColumnDescription& subColumnDescription)
    {
        subColumns << subColumnDescription;
    }

    void addSetting(const QString& key, const QString& value)
    {
        columnSettings.insert(key, value);
    }

    TableViewColumnConfiguration toConfiguration() const
    {
        TableViewColumnConfiguration configuration;

        configuration.columnId       = columnId;
        configuration.columnSettings = columnSettings;

        return configuration;
    }

    TableViewColumnDescription setIcon(const QString& iconName)
    {
        columnIcon = iconName;

        return *this;
    }

    static bool FindInListById(const TableViewColumnDescription::List& listToSearch, const QString& targetId, TableViewColumnDescription* const resultDescription)
    {
        TableViewColumnDescription::List leftToSearch = listToSearch;

        while (!leftToSearch.isEmpty())
        {
            const TableViewColumnDescription desc = leftToSearch.takeFirst();

            if (desc.columnId == targetId)
            {
                *resultDescription = desc;

                return true;
            }

            if (!desc.subColumns.isEmpty())
            {
                leftToSearch << desc.subColumns;
            }
        }

        return false;
    }
};

// ----------------------------------------------------------------------------

class TableViewColumnConfigurationWidget : public QWidget
{
    Q_OBJECT

public:

    explicit TableViewColumnConfigurationWidget(TableViewShared* const sharedObject,
                                                const TableViewColumnConfiguration& currentConfiguration,
                                                QWidget* const parent = nullptr);
    ~TableViewColumnConfigurationWidget() override = default;

    virtual TableViewColumnConfiguration getNewConfiguration() = 0;

public:

    TableViewShared* const       s = nullptr;
    TableViewColumnConfiguration configuration;
};

// ----------------------------------------------------------------------------

class TableViewColumn : public QObject
{
    Q_OBJECT

protected:

    TableViewShared* const       s = nullptr;
    TableViewColumnConfiguration configuration;

public:

    enum ColumnFlag
    {
        ColumnNoFlags                = 0,
        ColumnCustomPainting         = 1,
        ColumnCustomSorting          = 2,
        ColumnHasConfigurationWidget = 4
    };
    Q_DECLARE_FLAGS(ColumnFlags, ColumnFlag)

    enum ColumnCompareResult
    {
        CmpEqual                     = 0,
        CmpABiggerB                  = 1,
        CmpALessB                    = 2
    };

public:

    explicit TableViewColumn(TableViewShared* const tableViewShared,
                             const TableViewColumnConfiguration& pConfiguration,
                             QObject* const parent = nullptr);
    ~TableViewColumn() override = default;

public:

    virtual TableViewColumnConfiguration getConfiguration()                                                             const;
    virtual void setConfiguration(const TableViewColumnConfiguration& newConfiguration);
    virtual TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget)                     const;
    virtual ColumnFlags getColumnFlags()                                                                                const;
    virtual QString getTitle()                                                                                          const = 0;

    virtual QVariant data(TableViewModel::Item* const item, const int role)                                             const;
    virtual ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB)           const;
    virtual bool columnAffectedByChangeset(const ImageChangeset& imageChangeset)                                        const;
    virtual bool paint(QPainter* const painter, const QStyleOptionViewItem& option, TableViewModel::Item* const item)   const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, TableViewModel::Item* const item)                        const;
    virtual void updateThumbnailSize();

public:

    static bool compareHelperBoolFailCheck(const bool okA, const bool okB, ColumnCompareResult* const result);

    template<class MyType>
    static ColumnCompareResult compareHelper(const MyType& A, const MyType& B)
    {
        if      (A == B)
        {
            return CmpEqual;
        }
        else if (A > B)
        {
            return CmpABiggerB;
        }

        return CmpALessB;
    }

    template<typename columnClass>
    static bool getSubColumnIndex(const QString& subColumnId, typename columnClass::SubColumn* const subColumn)
    {
        const int index = columnClass::getSubColumns().indexOf(subColumnId);

        if (index < 0)
        {
            return false;
        }

        *subColumn = typename columnClass::SubColumn(index);

        return true;
    }

    template<typename columnClass>
    static bool CreateFromConfiguration(TableViewShared* const tableViewShared,
                                        const TableViewColumnConfiguration& pConfiguration,
                                        TableViewColumn** const pNewColumn,
                                        QObject* const parent)
    {
        typename columnClass::SubColumn subColumn;

        if (!getSubColumnIndex<columnClass>(pConfiguration.columnId, &subColumn))
        {
            return false;
        }

        *pNewColumn = new columnClass(tableViewShared, pConfiguration, subColumn, parent);

        return true;
    }

Q_SIGNALS:

    void signalDataChanged(const qlonglong imageId);
    void signalAllDataChanged();
};

// ----------------------------------------------------------------------------

class TableViewColumnFactory : public QObject
{
    Q_OBJECT

public:

    explicit TableViewColumnFactory(TableViewShared* const tableViewShared, QWidget* const parent);

    static QList<TableViewColumnDescription> getColumnDescriptionList();
    TableViewColumn* getColumn(const TableViewColumnConfiguration& columnConfiguration);

private:

    QWidget*         m_displayWidget = nullptr;
    TableViewShared* const s         = nullptr;
};

// ----------------------------------------------------------------------------

class TableViewColumnProfile
{
public:

    TableViewColumnProfile()  = default;
    ~TableViewColumnProfile() = default;

    void loadSettings(const KConfigGroup& configGroup);
    void saveSettings(KConfigGroup& configGroup);

public:

    QList<TableViewColumnConfiguration> columnConfigurationList;
    QString                             name;
    QByteArray                          headerState;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::TableViewColumnDescription)

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::TableViewColumn::ColumnFlags)
