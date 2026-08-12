/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to run user script.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "userscriptplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "userscript.h"

namespace DigikamBqmUserScriptPlugin
{

UserScriptPlugin::UserScriptPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString UserScriptPlugin::name() const
{
    return i18nc("@title", "User Shell Script");
}

QString UserScriptPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon UserScriptPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("text-x-script"));
}

QString UserScriptPlugin::description() const
{
    return i18nc("@info", "A tool to execute a custom shell script");
}

QString UserScriptPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can run user shell script as workflow stage.</para>"
                  "<para>Script variables:</para>"
                  "<para>Uses <b>$INPUT</b> and <b>$OUTPUT</b> for input / output filenames (with special characters escaped). "
                  "These would be substituted before shell execution.</para>"
                  "<para>Uses <b>$TITLE</b>, <b>$COMMENTS</b>, <b>$COLORLABEL</b>, <b>$PICKLABEL</b>, <b>$RATING</b> "
                  "and <b>$TAGSPATH</b> to handle digiKam properties.</para>");
}

QString UserScriptPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString UserScriptPlugin::handbookChapter() const
{
    return QLatin1String("custom_script");
}

QList<DPluginAuthor> UserScriptPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Hubert Law"),
                             QString::fromUtf8("hhclaw dot eb at gmail dot com"),
                             QString::fromUtf8("(C) 2014"))
            ;
}

void UserScriptPlugin::setup(QObject* const parent)
{
    UserScript* const tool = new UserScript(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmUserScriptPlugin

#include "moc_userscriptplugin.cpp"
