/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "welcomepageview_p.h"

namespace Digikam
{

WelcomePageView::WelcomePageView(QWidget* const parent)
    : QWidget(parent)
{
    QScrollArea* const sv = new QScrollArea(this);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sv->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    sv->setWidgetResizable(true);

    QWidget* const plain = new QWidget(sv->viewport());
    plain->setMinimumWidth(sv->width());
    plain->setContentsMargins(0, 0, 0, 0);

    // ---

    ResizableBackgroundWidget* const background = new ResizableBackgroundWidget(plain);
    GradientWidget* const gradHeader            = new GradientWidget(plain);

    // ---

    QWidget* const headerWidget = new QWidget(plain);
    QLabel* const logo          = new QLabel(headerWidget);
    logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                   QLatin1String("digikam/data/logo-digikam.webp")))
                    .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* const title         = new QLabel(QLatin1String("<qt><h1>digiKam</h1></qt>"), headerWidget);
    title->setObjectName(QLatin1String("app-name"));
    title->setStyleSheet(QLatin1String("#app-name {"
                                       "   color: rgba(255, 255, 255, 0.67);"
                                       "   margin-top: 11px;"
                                       "}"));
    title->setGraphicsEffect(new TitleEffect);

    QWidget* const titleWidget           = new QWidget(plain);

    QLabel* const smallTitle             = new QLabel(DAboutData::digiKamSlogan(), titleWidget);
    smallTitle->setAlignment(Qt::AlignCenter);
    smallTitle->setObjectName(QLatin1String("small-title"));
    smallTitle->setStyleSheet(QLatin1String("#small-title {"
                                            "   color: rgba(255, 255, 255, 0.67);"
                                            "   font-size: 14px;"
                                            "}"));
    smallTitle->setGraphicsEffect(new TitleEffect);

    QLabel* const bigTitle               = new QLabel(i18n("Welcome to digiKam %1",
                                                      QLatin1String(digikam_version)), titleWidget);
    bigTitle->setAlignment(Qt::AlignCenter);
    bigTitle->setObjectName(QLatin1String("big-title"));
    smallTitle->setStyleSheet(QLatin1String("#big-title {"
                                            "   color: rgba(255, 255, 255, 0.67);"
                                            "   font-size: 24px;"
                                            "   font-weight: bold;"
                                            "}"));
    bigTitle->setGraphicsEffect(new TitleEffect);

    QVBoxLayout* const titleLayout       = new QVBoxLayout(titleWidget);
    titleLayout->setContentsMargins(0, 0, 0, 20);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(smallTitle);
    titleLayout->addWidget(bigTitle);

    QHBoxLayout* const headerLayout      = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(10, 10, 10, 10);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(logo);
    headerLayout->addStretch(10);
    headerLayout->addWidget(titleWidget, Qt::AlignCenter);
    headerLayout->addStretch(10);
    headerLayout->addWidget(title);

    // ---

    InvertedGradientWidget* const gradFooter = new InvertedGradientWidget(plain);
    QWidget* const footerWidget              = new QWidget(plain);
    footerWidget->setContentsMargins(0, 0, 0, 0);
    footerWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // ---

    QWidget* const tabButtonsWidget      = new QWidget(plain);

    QPushButton* const newFeaturesButton = new QPushButton(i18n("New Features"), tabButtonsWidget);
    QPushButton* const aboutButton       = new QPushButton(i18n("About"), tabButtonsWidget);
    QPushButton* const creditsButton     = new QPushButton(i18n("Background Image Credits"), tabButtonsWidget);

    QString buttonStyle                  = QLatin1String("QPushButton { background-color: rgba(220, 220, 220, 128); border: none; border-radius: 8px; padding: 8px; }"
                                                         "QPushButton:checked { background-color: rgba(150, 150, 150, 200); }");
    newFeaturesButton->setStyleSheet(buttonStyle);
    newFeaturesButton->setAutoDefault(false);
    newFeaturesButton->setShortcutEnabled(false);
    newFeaturesButton->setFocusPolicy(Qt::NoFocus);
    newFeaturesButton->setShortcut(QKeySequence());
    aboutButton->setStyleSheet(buttonStyle);
    aboutButton->setAutoDefault(false);
    aboutButton->setShortcutEnabled(false);
    aboutButton->setFocusPolicy(Qt::NoFocus);
    aboutButton->setShortcut(QKeySequence());
    creditsButton->setStyleSheet(buttonStyle);
    creditsButton->setAutoDefault(false);
    creditsButton->setShortcutEnabled(false);
    creditsButton->setFocusPolicy(Qt::NoFocus);
    creditsButton->setShortcut(QKeySequence());

    // ---

    QHBoxLayout* const tabButtonsLayout  = new QHBoxLayout(tabButtonsWidget);
    tabButtonsLayout->setContentsMargins(0, 0, 0, 0);
    tabButtonsLayout->setSpacing(10);
    tabButtonsLayout->addStretch();
    tabButtonsLayout->addWidget(newFeaturesButton);
    tabButtonsLayout->addWidget(aboutButton);
    tabButtonsLayout->addWidget(creditsButton);
    tabButtonsLayout->addStretch();

    // ---

    QButtonGroup* const tabButtonGroup   = new QButtonGroup(plain);
    tabButtonGroup->addButton(newFeaturesButton);
    tabButtonGroup->addButton(aboutButton);
    tabButtonGroup->addButton(creditsButton);
    newFeaturesButton->setCheckable(true);
    aboutButton->setCheckable(true);
    creditsButton->setCheckable(true);

    // ---

    QStackedWidget* const stackedWidget  = new QStackedWidget(plain);
    stackedWidget->setContentsMargins(0, 0, 0, 0);
    QString labelsStyle                  = QLatin1String("background: rgba(240, 240, 240, 128); "
                                                         "color: black; border-radius: 15px; "
                                                         "padding: 10px;");

    QWidget* const newFeaturesTab        = new QWidget(stackedWidget);
    QLabel* const newFeatures            = new QLabel(featuresTabContent(), stackedWidget);
    newFeatures->setStyleSheet(labelsStyle);
    newFeatures->setOpenExternalLinks(true);
    newFeatures->setWordWrap(true);
    newFeatures->setMargin(10);
    QVBoxLayout* const vlay1             = new QVBoxLayout(newFeaturesTab);
    vlay1->addWidget(newFeatures);
    vlay1->addStretch();
    vlay1->setContentsMargins(80, 10, 80, 10);
    stackedWidget->addWidget(newFeaturesTab);

    QWidget* const aboutTab              = new QWidget(stackedWidget);
    QLabel* const about                  = new QLabel(aboutTabContent(), stackedWidget);
    about->setStyleSheet(labelsStyle);
    about->setOpenExternalLinks(true);
    about->setWordWrap(true);
    about->setMargin(10);
    QVBoxLayout* const vlay2             = new QVBoxLayout(aboutTab);
    vlay2->addWidget(about);
    vlay2->addStretch();
    vlay2->setContentsMargins(80, 10, 80, 10);
    stackedWidget->addWidget(aboutTab);

    QWidget* const creditsTab            = new QWidget(stackedWidget);
    QLabel* const credits                = new QLabel(creditsTabContent(), stackedWidget);
    credits->setStyleSheet(labelsStyle);
    credits->setOpenExternalLinks(true);
    credits->setWordWrap(true);
    credits->setMargin(10);
    QVBoxLayout* const vlay3             = new QVBoxLayout(creditsTab);
    vlay3->addWidget(credits);
    vlay3->addStretch();
    vlay3->setContentsMargins(80, 10, 80, 10);
    stackedWidget->addWidget(creditsTab);

    // ---

    connect(newFeaturesButton, &QPushButton::clicked,
            this, [stackedWidget, newFeaturesTab]()
        {
            stackedWidget->setCurrentWidget(newFeaturesTab);
        }
    );

    connect(aboutButton, &QPushButton::clicked,
            this, [stackedWidget, aboutTab]()
        {
            stackedWidget->setCurrentWidget(aboutTab);
        }
    );

    connect(creditsButton, &QPushButton::clicked,
            this, [stackedWidget, creditsTab]()
        {
            stackedWidget->setCurrentWidget(creditsTab);
        }
    );

    aboutButton->click();

    // ---

    QGridLayout* const grid = new QGridLayout(plain);
    grid->addWidget(gradHeader,       0, 0, 1, 3);
    grid->addWidget(headerWidget,     0, 0, 1, 3);
    grid->addWidget(background,       1, 0, 2, 3);
    grid->addWidget(tabButtonsWidget, 1, 0, 1, 3, Qt::AlignCenter);
    grid->addWidget(gradFooter,       3, 0, 1, 3);
    grid->addWidget(footerWidget,     3, 0, 1, 3);
    grid->addWidget(stackedWidget,    2, 0, 2, 3);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setRowStretch(3, 10);

    // ---

    sv->setWidget(plain);

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(sv);
}

QString WelcomePageView::appendSubList(const QStringList& list) const
{
    QString sub;

    for (const QString& str : list)
    {
        sub.append(QLatin1String("<ul>") + str + QLatin1String("</ul>"));
    }

    return sub;
}

} // namespace Digikam

#include "moc_welcomepageview.cpp"
