/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "editortoolsettings.h"

// Qt includes

#include <QButtonGroup>
#include <QLabel>
#include <QPixmap>
#include <QLayout>
#include <QMap>
#include <QPair>
#include <QString>
#include <QToolButton>
#include <QVariant>
#include <QScrollBar>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "dnuminput.h"
#include "colorgradientwidget.h"
#include "histogramwidget.h"
#include "dcolorselector.h"
#include "dpluginaboutdlg.h"
#include "dplugineditor.h"
#include "editortool.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN EditorToolSettings::Private
{
public:

    Private() = default;

public:

    QButtonGroup*        scaleBG            = nullptr;

    QToolButton*         linHistoButton     = nullptr;
    QToolButton*         logHistoButton     = nullptr;

    QWidget*             settingsArea       = nullptr;
    QWidget*             plainPage          = nullptr;

    QLabel*              toolName           = nullptr;
    QLabel*              toolIcon           = nullptr;
    QPushButton*         toolHelp           = nullptr;
    QPushButton*         toolAbout          = nullptr;

    DHBox*               guideBox           = nullptr;

    QPushButton*         okBtn              = nullptr;
    QPushButton*         cancelBtn          = nullptr;
    QPushButton*         tryBtn             = nullptr;
    QPushButton*         defaultBtn         = nullptr;
    QPushButton*         saveAsBtn          = nullptr;
    QPushButton*         loadBtn            = nullptr;

    DColorSelector*      guideColorBt       = nullptr;

    ColorGradientWidget* hGradient          = nullptr;

    HistogramBox*        histogramBox       = nullptr;

    DIntNumInput*        guideSize          = nullptr;

    EditorTool*          tool               = nullptr;
};

EditorToolSettings::EditorToolSettings(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);

    d->settingsArea                 = new QWidget;

    // ---------------------------------------------------------------

    QGridLayout* const gridSettings = new QGridLayout(d->settingsArea);
    d->plainPage                    = new QWidget;
    d->guideBox                     = new DHBox;
    d->histogramBox                 = new HistogramBox;

    // ---------------------------------------------------------------

    QFrame* const toolDescriptor    = new QFrame;
    d->toolName                     = new QLabel();
    d->toolIcon                     = new QLabel();
    QFont font                      = d->toolName->font();
    font.setBold(true);
    d->toolName->setFont(font);

    d->toolAbout                    = new QPushButton();
    d->toolAbout->setIcon(QIcon::fromTheme(QLatin1String("help-about")));
    d->toolAbout->setToolTip(i18n("About this tool..."));

    d->toolHelp                     = new QPushButton();
    d->toolHelp->setIcon(QIcon::fromTheme(QLatin1String("globe")));
    d->toolHelp->setToolTip(i18n("Tool online handbook..."));

    QString frameStyle = QString::fromLatin1("QFrame {"
                                             "color: %1;"
                                             "border: 1px solid %2;"
                                             "border-radius: 5px;"
                                             "background-color: %3;"
                                             "}")
                         .arg(QApplication::palette().color(QPalette::HighlightedText).name())
                         .arg(QApplication::palette().color(QPalette::HighlightedText).name())
                         .arg(QApplication::palette().color(QPalette::Highlight).name());

    QString noFrameStyle(QLatin1String("QFrame {"
                                       "border: none;"
                                       "}"));

    toolDescriptor->setStyleSheet(frameStyle);
    d->toolName->setStyleSheet(noFrameStyle);
    d->toolIcon->setStyleSheet(noFrameStyle);
    d->toolHelp->setStyleSheet(noFrameStyle);
    d->toolAbout->setStyleSheet(noFrameStyle);

    QGridLayout* const descrLayout = new QGridLayout();
    descrLayout->addWidget(d->toolIcon,  0, 0, 1, 1);
    descrLayout->addWidget(d->toolName,  0, 1, 1, 1);
    descrLayout->addWidget(d->toolHelp,  0, 2, 1, 1);
    descrLayout->addWidget(d->toolAbout, 0, 3, 1, 1);
    descrLayout->setColumnStretch(1, 10);
    toolDescriptor->setLayout(descrLayout);

    // ---------------------------------------------------------------

    new QLabel(i18n("Guide:"), d->guideBox);
    QLabel* const space4 = new QLabel(d->guideBox);
    d->guideColorBt      = new DColorSelector(d->guideBox);
    d->guideColorBt->setColor(QColor(Qt::red));
    d->guideColorBt->setWhatsThis(i18n("Set here the color used to draw dashed guide lines."));
    d->guideSize         = new DIntNumInput(d->guideBox);
    d->guideSize->setSuffix(QLatin1String("px"));
    d->guideSize->setRange(1, 5, 1);
    d->guideSize->setDefaultValue(1);
    d->guideSize->setWhatsThis(i18n("Set here the width in pixels used to draw dashed guide lines."));

    d->guideBox->setStretchFactor(space4, 10);
    d->guideBox->setContentsMargins(QMargins());
    d->guideBox->setSpacing(spacingHint());

    // ---------------------------------------------------------------

    d->defaultBtn = new QPushButton(i18n("Defaults"));
    d->defaultBtn->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->defaultBtn->setToolTip(i18n("Reset all settings to their default values."));

    d->okBtn      = new QPushButton(i18n("OK"));
    d->okBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->okBtn->setDefault(true);

    d->cancelBtn  = new QPushButton(i18n("Cancel"));
    d->cancelBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));

    QHBoxLayout* const hbox1 = new QHBoxLayout;
    hbox1->addWidget(d->defaultBtn);
    hbox1->addStretch(1);
    hbox1->addWidget(d->okBtn);
    hbox1->addWidget(d->cancelBtn);

    // ---------------------------------------------------------------

    d->loadBtn = new QPushButton(i18nc("@action: load previously saved filter settings", "Load..."));
    d->loadBtn->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
    d->loadBtn->setToolTip(i18n("Load all parameters from settings text file."));

    d->saveAsBtn = new QPushButton(i18nc("@action: save current filter settings", "Save As..."));
    d->saveAsBtn->setIcon(QIcon::fromTheme(QLatin1String("document-save-as")));
    d->saveAsBtn->setToolTip(i18n("Save all parameters to settings text file."));

    d->tryBtn = new QPushButton(i18nc("@action: try current filter settings", "Try"));
    d->tryBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->tryBtn->setToolTip(i18n("Try all settings."));

    QHBoxLayout* const hbox2 = new QHBoxLayout;
    hbox2->addWidget(d->loadBtn);
    hbox2->addWidget(d->saveAsBtn);
    hbox2->addStretch(1);
    hbox2->addWidget(d->tryBtn);

    // ---------------------------------------------------------------

    const int spacing = spacingHint();

    gridSettings->addWidget(toolDescriptor,  0, 0, 1, -1);
    gridSettings->addWidget(d->histogramBox, 1, 0, 2, 2);
    gridSettings->addWidget(d->plainPage,    4, 0, 1, 2);
    gridSettings->addWidget(d->guideBox,     5, 0, 1, 2);
    gridSettings->addLayout(hbox2,           6, 0, 1, 2);
    gridSettings->addLayout(hbox1,           7, 0, 1, 2);
    gridSettings->setContentsMargins(spacing, spacing, spacing, spacing);
    gridSettings->setSpacing(spacing);

    // ---------------------------------------------------------------

    setWidget(d->settingsArea);

    // ---------------------------------------------------------------

    connect(d->okBtn, SIGNAL(clicked()),
            this, SIGNAL(signalOkClicked()));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SIGNAL(signalCancelClicked()));

    connect(d->tryBtn, SIGNAL(clicked()),
            this, SIGNAL(signalTryClicked()));

    connect(d->defaultBtn, SIGNAL(clicked()),
            this, SIGNAL(signalDefaultClicked()));

    connect(d->saveAsBtn, SIGNAL(clicked()),
            this, SIGNAL(signalSaveAsClicked()));

    connect(d->loadBtn, SIGNAL(clicked()),
            this, SIGNAL(signalLoadClicked()));

    connect(d->guideColorBt, SIGNAL(signalColorSelected(QColor)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->guideSize, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->histogramBox, SIGNAL(signalChannelChanged(ChannelType)),
            this, SIGNAL(signalChannelChanged()));

    connect(d->histogramBox, SIGNAL(signalScaleChanged(HistogramScale)),
            this, SIGNAL(signalScaleChanged()));

    connect(d->toolAbout, SIGNAL(clicked()),
            this, SLOT(slotAboutPlugin()));

    connect(d->toolHelp, SIGNAL(clicked()),
            this, SLOT(slotHelpPlugin()));

    // --------------------------------------------------------

    setTabOrder(d->tryBtn,     d->okBtn);
    setTabOrder(d->okBtn,      d->cancelBtn);
    setTabOrder(d->cancelBtn,  d->defaultBtn);
    setTabOrder(d->defaultBtn, d->loadBtn);
    setTabOrder(d->loadBtn,    d->saveAsBtn);

    // ---------------------------------------------------------------

    setButtons(Default | Ok | Cancel);
    setTools(NoTool);
}

EditorToolSettings::~EditorToolSettings()
{
    delete d;
}

QSize EditorToolSettings::minimumSizeHint() const
{
    // Editor Tools usually require a larger horizontal space than other widgets in right side bar
    // Set scroll area to a horizontal minimum size sufficient for the settings.
    // Do not touch vertical size hint.
    // Limit to 40% of the desktop width.

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = d->settingsArea->nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QRect desktopRect = screen->geometry();
    QSize hint        = QScrollArea::minimumSizeHint();
    int wSB           = verticalScrollBar()->sizeHint().width();
    hint.setWidth(qMin(d->settingsArea->sizeHint().width() + wSB, desktopRect.width() * 2 / 5));

    return hint;
}

int EditorToolSettings::marginHint()
{
    return layoutMargin();
}

int EditorToolSettings::spacingHint()
{
    return layoutSpacing();
}

QWidget* EditorToolSettings::plainPage() const
{
    return d->plainPage;
}

HistogramBox* EditorToolSettings::histogramBox() const
{
    return d->histogramBox;
}

QPushButton* EditorToolSettings::button(int buttonCode) const
{
    if (buttonCode & Default)
    {
        return d->defaultBtn;
    }

    if (buttonCode & Try)
    {
        return d->tryBtn;
    }

    if (buttonCode & Ok)
    {
        return d->okBtn;
    }

    if (buttonCode & Cancel)
    {
        return d->cancelBtn;
    }

    if (buttonCode & Load)
    {
        return d->loadBtn;
    }

    if (buttonCode & SaveAs)
    {
        return d->saveAsBtn;
    }

    return nullptr;
}

void EditorToolSettings::enableButton(int buttonCode, bool state)
{
    QPushButton* const btn = button(buttonCode);

    if (btn)
    {
        btn->setEnabled(state);
    }
}

QColor EditorToolSettings::guideColor() const
{
    return d->guideColorBt->color();
}

void EditorToolSettings::setGuideColor(const QColor& color)
{
    d->guideColorBt->setColor(color);
}

int EditorToolSettings::guideSize() const
{
    return d->guideSize->value();
}

void EditorToolSettings::setGuideSize(int size)
{
    d->guideSize->setValue(size);
}

void EditorToolSettings::setButtons(Buttons buttonMask)
{
    d->okBtn->setVisible(buttonMask & Ok);
    d->cancelBtn->setVisible(buttonMask & Cancel);
    d->defaultBtn->setVisible(buttonMask & Default);

    d->loadBtn->setVisible(buttonMask & Load);
    d->saveAsBtn->setVisible(buttonMask & SaveAs);
    d->tryBtn->setVisible(buttonMask & Try);
}

void EditorToolSettings::setTools(Tools toolMask)
{
    d->histogramBox->setVisible(toolMask & Histogram);
    d->guideBox->setVisible(toolMask & ColorGuide);
}

void EditorToolSettings::setHistogramType(HistogramBoxType type)
{
    d->histogramBox->setHistogramType(type);
}

void EditorToolSettings::setTool(EditorTool* const tool)
{
    d->tool = tool;
    d->toolName->setText(d->tool->toolName());
    d->toolIcon->setPixmap(d->tool->toolIcon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
}

void EditorToolSettings::slotAboutPlugin()
{
    if (d->tool)
    {
        if (d->tool->plugin())
        {
            QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(d->tool->plugin());
            dialogExec(dlg);

            delete dlg;
        }
    }
}

void EditorToolSettings::slotHelpPlugin()
{
    if (d->tool)
    {
        if (d->tool->plugin())
        {
            openOnlineDocumentation(d->tool->plugin()->handbookSection(),
                                    d->tool->plugin()->handbookChapter(),
                                    d->tool->plugin()->handbookReference());
        }
    }
}

} // namespace Digikam

#include "moc_editortoolsettings.cpp"
