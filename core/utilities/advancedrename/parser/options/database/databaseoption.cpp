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

#include "databaseoption.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QLineEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "itemcomments.h"
#include "dbkeyselector.h"
#include "digikam_debug.h"
#include "commonkeys.h"
#include "metadatakeys.h"
#include "positionkeys.h"

namespace Digikam
{

DatabaseOptionDialog::DatabaseOptionDialog(Rule* const parent, QWidget* const widget)
    : RuleDialog(parent, widget)
{
    QWidget* const mainWidget = new QWidget(this);
    dbkeySelectorView         = new DbKeySelectorView(this);
    QLabel* const customLabel = new QLabel(i18n("Keyword separator:"));
    separatorLineEdit         = new QLineEdit(this);
    separatorLineEdit->setText(QLatin1String("_"));

    // --------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout(this);
    mainLayout->addWidget(customLabel,       0, 0, 1, 1);
    mainLayout->addWidget(separatorLineEdit, 0, 1, 1, 1);
    mainLayout->addWidget(dbkeySelectorView, 1, 0, 1, -1);
    mainWidget->setLayout(mainLayout);

    // --------------------------------------------------------

    setSettingsWidget(mainWidget);
    resize(450, 450);
}

// --------------------------------------------------------

DatabaseOption::DatabaseOption(QWidget* const widget)
    : Option(i18n("Database..."),
             i18n("Add information from the database"),
             QLatin1String("network-server-database"))
{
    addToken(QLatin1String("[db:||key||]"), i18n("Add database information"));
    QRegularExpression reg(QLatin1String("\\[db(:(.*))\\]"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
    setParentWidget(widget);

    registerKeysCollection();
}

DatabaseOption::~DatabaseOption()
{
    unregisterKeysCollection();
}

void DatabaseOption::registerKeysCollection()
{
    addDbKeysCollection(new CommonKeys());
    addDbKeysCollection(new MetadataKeys());
    addDbKeysCollection(new PositionKeys());
}

void DatabaseOption::unregisterKeysCollection()
{
    QSet<DbKeysCollection*> alreadyDeleted;

    for (DbKeysCollection* const key : std::as_const(m_map))
    {
        if (key && !alreadyDeleted.contains(key))
        {
            alreadyDeleted.insert(key);
            delete key;
        }
    }

    m_map.clear();
}

void DatabaseOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QStringList keys;
    QPointer<DatabaseOptionDialog> dlg = new DatabaseOptionDialog(this, getParentWidget());
    dlg->dbkeySelectorView->setKeysMap(m_map);

    if (dialogExec(dlg) == QDialog::Accepted)
    {
        QStringList checkedKeys = dlg->dbkeySelectorView->checkedKeysList();

        for (const QString& key : std::as_const(checkedKeys))
        {
            QString keyStr = QString::fromUtf8("[db:%1]").arg(key);
            keys << keyStr;
        }
    }

    if (!keys.isEmpty())
    {
        QString tokenStr = keys.join(dlg->separatorLineEdit->text());
        Q_EMIT signalTokenTriggered(tokenStr);
    }

    delete dlg;
}

QString DatabaseOption::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    QString keyword = match.captured(2);

    return parseDatabase(keyword, settings);
}

QString DatabaseOption::parseDatabase(const QString& keyword, ParseSettings& settings)
{
    if (settings.fileUrl.isEmpty() || keyword.isEmpty())
    {
        return QString();
    }

    DbKeysCollection* const dbkey = m_map.value(keyword);

    if (!dbkey)
    {
        return QString();
    }

    return dbkey->getValue(keyword, settings);
}

void DatabaseOption::addDbKeysCollection(DbKeysCollection* key)
{
    if (!key)
    {
        return;
    }

    DbKeyIdsMap map = key->ids();

    for (DbKeyIdsMap::const_iterator it = map.constBegin() ;
         it != map.constEnd() ; ++it)
    {
        m_map.insert(it.key(), key);
    }
}

} // namespace Digikam

#include "moc_databaseoption.cpp"
