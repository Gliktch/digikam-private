/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : an option to provide database information to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QMap>

// Local includes

#include "option.h"
#include "ruledialog.h"

class QLineEdit;

namespace Digikam
{
class DbKeysCollection;
class DbKeySelectorView;

class DatabaseOptionDialog : public RuleDialog
{
    Q_OBJECT

public:

    explicit DatabaseOptionDialog(Rule* const parent, QWidget* const widget);
    ~DatabaseOptionDialog() override = default;

    DbKeySelectorView* dbkeySelectorView    = nullptr;
    QLineEdit*         separatorLineEdit    = nullptr;

private:

    /// @note disabled
    explicit DatabaseOptionDialog(QWidget*)                      = delete;
    DatabaseOptionDialog(const DatabaseOptionDialog&)            = delete;
    DatabaseOptionDialog& operator=(const DatabaseOptionDialog&) = delete;
};

// --------------------------------------------------------

typedef QMap<QString, DbKeysCollection*> DbOptionKeysMap;

// --------------------------------------------------------

class DatabaseOption : public Option
{
    Q_OBJECT

public:

    explicit DatabaseOption(QWidget* const widget);
    ~DatabaseOption()                                            override;

protected:

    QString parseOperation(ParseSettings& settings,
                           const QRegularExpressionMatch& match) override;

private Q_SLOTS:

    void slotTokenTriggered(const QString& token)                override;

private:

    QString parseDatabase(const QString& keyword, ParseSettings& settings);
    void addDbKeysCollection(DbKeysCollection* key);

    void registerKeysCollection();
    void unregisterKeysCollection();

private:

    DbOptionKeysMap m_map;

private:

    /// @note disabled
    explicit DatabaseOption(QObject*)                = delete;
    DatabaseOption(const DatabaseOption&)            = delete;
    DatabaseOption& operator=(const DatabaseOption&) = delete;
};

} // namespace Digikam
