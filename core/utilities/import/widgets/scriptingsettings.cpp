/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-20
 * Description : scripting settings for camera interface.
 *
 * SPDX-FileCopyrightText: 2012 by Petri Damstén <damu@iki.fi>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "scriptingsettings.h"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dfileselector.h"
#include "digikam_debug.h"
#include "tooltipdialog.h"

namespace Digikam
{

class Q_DECL_HIDDEN ScriptingSettings::Private
{
public:

    Private() = default;

    QLabel*        scriptLabel          = nullptr;
    DFileSelector* script               = nullptr;
    TooltipDialog* tooltipDialog        = nullptr;
    QToolButton*   tooltipToggleButton  = nullptr;
};

ScriptingSettings::ScriptingSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing = layoutSpacing();


    d->tooltipDialog  = new TooltipDialog(this);
    d->tooltipDialog->setTooltip(i18n("<p>These expressions may be used to customize the command line:</p>"
                                      "<p><b>%file</b>: full path of the imported file</p>"
                                      "<p><b>%filename</b>: file name of the imported file</p>"
                                      "<p><b>%path</b>: path of the imported file</p>"
                                      "<p><b>%orgfilename</b>: original file name</p>"
                                      "<p><b>%orgpath</b>: original path</p>"
                                      "<p>If there are no expressions full path is added to the command.</p>"
                                     ));
    d->tooltipDialog->resize(650, 530);

    QVBoxLayout* const vlay = new QVBoxLayout(this);
    d->scriptLabel          = new QLabel(i18n("Execute script for image:"), this);
    DHBox* const hbox       = new DHBox(this);
    d->script               = new DFileSelector(hbox);
    d->script->setFileDlgMode(QFileDialog::ExistingFile);
    d->script->lineEdit()->setPlaceholderText(i18n("No script selected"));
    d->tooltipToggleButton  = new QToolButton(hbox);
    d->tooltipToggleButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->tooltipToggleButton->setToolTip(i18n("Show a list of all available options"));

    vlay->addWidget(d->scriptLabel);
    vlay->addWidget(hbox);
    vlay->addStretch();
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    setWhatsThis(i18n("Set here the script that is executed for every imported image."));

    // ---------------------------------------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(clicked(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));
}

ScriptingSettings::~ScriptingSettings()
{
    delete d;
}

void ScriptingSettings::readSettings(const KConfigGroup& group)
{
    d->script->setFileDlgPath(group.readEntry("Script", QString()));
}

void ScriptingSettings::saveSettings(KConfigGroup& group)
{
    group.writeEntry("Script", d->script->fileDlgPath());
}

void ScriptingSettings::settings(DownloadSettings* const settings) const
{
    settings->script = d->script->fileDlgPath();
}

void ScriptingSettings::slotToolTipButtonToggled(bool /*checked*/)
{
    if (!d->tooltipDialog->isVisible())
    {
        d->tooltipDialog->show();
    }

    d->tooltipDialog->raise();
}

} // namespace Digikam

#include "moc_scriptingsettings.cpp"
