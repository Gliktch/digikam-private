/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a class that manages the files to be renamed
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QString>
#include <QStringList>
#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class AdvancedRenameWidget;
class Parser;
class ParseSettings;

class DIGIKAM_GUI_EXPORT AdvancedRenameManager : public QObject
{
    Q_OBJECT

public:

    enum ParserType
    {
        DefaultParser = 0,
        ImportParser
    };

    enum SortAction
    {
        SortName = 0,
        SortDate,
        SortSize,
        SortCustom
    };

    enum SortDirection
    {
        SortAscending = 0,
        SortDescending
    };

public:

    AdvancedRenameManager();
    explicit AdvancedRenameManager(const QList<ParseSettings>& files);
    ~AdvancedRenameManager()                                                  override;

    void addFiles(const QList<ParseSettings>& files);
    void reset();

    void parseFiles();
    void parseFiles(const ParseSettings& settings);
    void parseFiles(const QString& parseString);
    void parseFiles(const QString& parseString, const ParseSettings& settings);

    void setParserType(ParserType type);
    Parser* getParser()                                                 const;

    void setSortAction(SortAction action);
    SortAction sortAction()                                             const;

    void setSortDirection(SortDirection direction);
    SortDirection sortDirection()                                       const;

    void setStartIndex(int index);
    void setCutFileName(int index);

    void setWidget(AdvancedRenameWidget* widget);

    int indexOfFile(const QString& filename);
    int indexOfFolder(const QString& filename);
    int indexOfFileGroup(const QString& filename);
    int indexOfFileCounter(const QString& filename);

    QString randomStringOfIndex(int index);
    QString newName(const QString& filename)                            const;

    QStringList            fileList()                                   const;
    QMap<QString, QString> newFileList(bool checkFileSystem = false)    const;

Q_SIGNALS:

    void signalSortingChanged(QList<QUrl>);

private:

    void addFile(const QString& filename)                               const;
    void addFile(const QString& filename, const QDateTime& datetime)    const;
    bool initialize();
    void initializeFileList();
    void resetState();

    QString fileGroupKey(const QString& filename)                       const;

    void clearMappings();
    void clearAll();

private:

    /// @note disabled
    AdvancedRenameManager(QObject*)                                = delete;
    AdvancedRenameManager(const AdvancedRenameManager&)            = delete;
    AdvancedRenameManager& operator=(const AdvancedRenameManager&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
