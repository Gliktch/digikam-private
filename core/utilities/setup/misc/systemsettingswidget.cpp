/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-27-07
 * Description : system settings widget
 *
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 * SPDX-FileCopyrightText: 2025      by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "systemsettingswidget.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QStyle>
#include <QUrl>
#include <QPointer>
#include <QMessageBox>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "systemsettings.h"
#include "filesdownloader.h"
#include "dexpanderbox.h"
#include "ocvocldnntestdlg.h"
#include "ui_proxysettingswidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN SystemSettingsWidget::Private
{

public:

    Private() = default;

public:

    // Screen options

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    QCheckBox*              useHighDpiScalingCheck = nullptr;
    QCheckBox*              useHighDpiPixmapsCheck = nullptr;

#endif

    QCheckBox*              softwareOpenGLCheck    = nullptr;

    // OpenCV and AI options

    QCheckBox*              enableOpenCLCheck      = nullptr;
    QCheckBox*              enableOpenCLDNNCheck   = nullptr;
    QPushButton*            openCLDNNTest          = nullptr;
    OpenCVOpenCLDNNTestDlg* openCLDNNTestDlg       = nullptr;
    bool                    openCLDNNTestResult    = false;
    QPushButton*            filesDownloadButton    = nullptr;
    QPushButton*            openLocalStorageButton = nullptr;

    // Video rendering options

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QCheckBox*              enableHWVideoCheck     = nullptr;
    QCheckBox*              enableHWTConvCheck     = nullptr;

    QComboBox*              videoBackendCBox       = nullptr;

#endif

    // Debug traces options

    QCheckBox*              enableLoggingCheck     = nullptr;

    // Proxy settings options

    Ui::ProxySettingsWidget uiProxySettings;
};

SystemSettingsWidget::SystemSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing         = layoutSpacing();
    QGridLayout* const layout = new QGridLayout(this);

    // ---

    QLabel* const systemNote  = new QLabel(i18nc("@info",
                                                 "<font color='black'><i><b>⚠️Warning:</b> "
                                                 "All changes to these settings only take effect "
                                                 "after the restart. Some settings are hardware dependent "
                                                 "and may have side-effects. Read the "
                                                 "<a href=\"%1\">online documentation</a> for more information.</i></font>",
                                                 QString::fromUtf8("https://docs.digikam.org/en/setup_application/miscs_settings.html#system-settings")),
                                           this);
    systemNote->setOpenExternalLinks(true);
    systemNote->setTextFormat(Qt::RichText);
    systemNote->setFocusPolicy(Qt::NoFocus);
    systemNote->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    systemNote->setWordWrap(true);
    systemNote->setAlignment(Qt::AlignCenter);
    systemNote->setStyleSheet(QLatin1String("QLabel { background-color: #ffcccc; border: none; padding: 5px; }"));

    // Screen options

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    d->useHighDpiScalingCheck = new QCheckBox(i18n("Use high DPI scaling from the screen factor"), this);
    d->useHighDpiPixmapsCheck = new QCheckBox(i18n("Use pixmaps with high DPI resolution"), this);

#endif

    d->softwareOpenGLCheck    = new QCheckBox(i18n("Use software OpenGL for rendering"), this);

    // OpenCV and AI options

    d->enableOpenCLCheck      = new QCheckBox(i18n("Use OpenCL hardware acceleration"), this);
    d->enableOpenCLCheck->setToolTip(i18n("This option is still experimental and "
                                          "requires that certain environment variables are set manually."));
    d->enableOpenCLDNNCheck   = new QCheckBox(i18n("Use OpenCL acceleration for AI models"), this);
    d->enableOpenCLDNNCheck->setToolTip(i18n("This option is still experimental and "
                                             "may lead to crashes if the proper drivers are not installed."));

    d->openCLDNNTest          = new QPushButton(i18n("Test GPU AI compatibility"), this);
    d->openCLDNNTest->setIcon(QIcon::fromTheme(QLatin1String("show-gpu-effects")));

    QLabel* const filesLabel  = new QLabel(i18n("Download required binary data:"), this);
    d->filesDownloadButton    = new QPushButton(i18n("Open Download Dialog..."), this);
    d->filesDownloadButton->setIcon(QIcon::fromTheme(QLatin1String("download")));

    d->openLocalStorageButton = new QPushButton(i18n("Open Local Storage..."), this);
    d->openLocalStorageButton->setIcon(QIcon::fromTheme(QLatin1String("folder-open")));
    d->openLocalStorageButton->setToolTip(i18n("Open local directory used for the data files storage."));

    // Video rendering options

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->enableHWVideoCheck     = new QCheckBox(i18n("Use video hardware acceleration"), this);
    d->enableHWTConvCheck     = new QCheckBox(i18n("Use video textures conversion"), this);

    QLabel* const videoLabel  = new QLabel(i18n("Decoding backend to render media:"), this);

    d->videoBackendCBox       = new QComboBox(this);
    d->videoBackendCBox->addItem(i18n("FFmpeg (Default)"),      QLatin1String("ffmpeg"));

#   if defined(Q_OS_LINUX)

    d->videoBackendCBox->addItem(i18n("GStreamer (System)"),    QLatin1String("gstreamer"));

#   elif defined(Q_OS_WIN)

    d->videoBackendCBox->addItem(i18n("WMF (System)"),          QLatin1String("windows"));

#   elif defined(Q_OS_MACOS)

    d->videoBackendCBox->addItem(i18n("AVFoundation (System)"), QLatin1String("darwin"));

#   endif

#endif

    // Debug traces options

    d->enableLoggingCheck     = new QCheckBox(i18n("Enable internal debug logging"), this);

    // Proxy settings options

    QWidget* const proxySettings = new QWidget(this);
    d->uiProxySettings.setupUi(proxySettings);
    proxySettings->setContentsMargins(QMargins());

    // ---

    if (qApp->applicationName() == QLatin1String("showfoto"))
    {
        d->enableOpenCLCheck->hide();
        d->enableOpenCLDNNCheck->hide();
        d->openCLDNNTest->hide();
    }

    // ---

    int row = 0;

    layout->addWidget(systemNote,                            row++, 0, 1, 3);

    // Screen options

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    layout->addWidget(d->useHighDpiScalingCheck,             row++, 0, 1, 2);
    layout->addWidget(d->useHighDpiPixmapsCheck,             row++, 0, 1, 2);

#endif

    layout->addWidget(d->softwareOpenGLCheck,                row++, 0, 1, 2);

    // OpenCV and AI options

    layout->addWidget(new DLineWidget(Qt::Horizontal, this), row++, 0, 1, 3);
    layout->addWidget(d->enableOpenCLCheck,                  row++, 0, 1, 2);
    layout->addWidget(d->enableOpenCLDNNCheck,               row,   0, 1, 1);
    layout->addWidget(d->openCLDNNTest,                      row++, 1, 1, 2);
    layout->addWidget(filesLabel,                            row,   0, 1, 1);
    layout->addWidget(d->filesDownloadButton,                row,   1, 1, 1);
    layout->addWidget(d->openLocalStorageButton,             row++, 2, 1, 1);

    // Video rendering options

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    layout->addWidget(new DLineWidget(Qt::Horizontal, this), row++, 0, 1, 3);
    layout->addWidget(d->enableHWVideoCheck,                 row++, 0, 1, 3);
    layout->addWidget(d->enableHWTConvCheck,                 row++, 0, 1, 3);
    layout->addWidget(videoLabel,                            row,   0, 1, 1);
    layout->addWidget(d->videoBackendCBox,                   row++, 1, 1, 2);

#endif

    // Debug traces options

    layout->addWidget(new DLineWidget(Qt::Horizontal, this), row++, 0, 1, 3);
    layout->addWidget(d->enableLoggingCheck,                 row++, 0, 1, 3);

    // Proxy settings options

    layout->addWidget(new DLineWidget(Qt::Horizontal, this), row++, 0, 1, 3);
    layout->addWidget(proxySettings,                         row++, 0, 1, 3);
    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setRowStretch(row, 10);

    connect(d->filesDownloadButton, &QPushButton::pressed,
            this, &SystemSettingsWidget::slotBinaryDownload);

    connect(d->openLocalStorageButton, &QPushButton::pressed,
            this, &SystemSettingsWidget::slotOpenLocalStorage);

    connect(d->enableOpenCLCheck, &QCheckBox::toggled,
            d->enableOpenCLDNNCheck, &QCheckBox::setEnabled);

    connect(d->enableOpenCLCheck, &QCheckBox::toggled,
            d->openCLDNNTest, &QPushButton::setEnabled);

    connect(d->openCLDNNTest, &QPushButton::pressed,
            this, &SystemSettingsWidget::slotOpenCLDNNTest);
}

SystemSettingsWidget::~SystemSettingsWidget()
{
    delete d;
}

void SystemSettingsWidget::readSettings()
{
    SystemSettings system(qApp->applicationName());

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    d->useHighDpiScalingCheck->setChecked(system.useHighDpiScaling);
    d->useHighDpiPixmapsCheck->setChecked(system.useHighDpiPixmaps);

#endif

    d->softwareOpenGLCheck->setChecked(system.softwareOpenGL);
    d->enableLoggingCheck->setChecked(system.enableLogging);
    d->enableOpenCLCheck->setChecked(system.enableOpenCL);
    d->enableOpenCLDNNCheck->setChecked(system.enableDnnOpenCL && system.enableOpenCL);
    d->enableOpenCLDNNCheck->setEnabled(system.enableOpenCL && system.dnnOpenCLTested);
    d->openCLDNNTest->setEnabled(system.enableOpenCL);
    d->openCLDNNTestResult = system.dnnOpenCLTested;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->enableHWVideoCheck->setChecked(system.enableHWVideo);
    d->enableHWTConvCheck->setChecked(system.enableHWTConv);
    d->videoBackendCBox->setCurrentIndex(d->videoBackendCBox->findData(system.videoBackend));

#endif

    // Proxy Settings

    d->uiProxySettings.kcfg_proxyUrl->setText(system.proxyUrl);
    d->uiProxySettings.kcfg_proxyPort->setValue(system.proxyPort);
    d->uiProxySettings.kcfg_proxyUser->setText(system.proxyUser);
    d->uiProxySettings.kcfg_proxyPass->setText(system.proxyPass);
    d->uiProxySettings.kcfg_proxyType->setCurrentIndex(system.proxyType);
    d->uiProxySettings.kcfg_proxyAuth->setChecked(system.proxyAuth);
}

void SystemSettingsWidget::saveSettings()
{
    SystemSettings system(qApp->applicationName());

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    system.useHighDpiScaling = d->useHighDpiScalingCheck->isChecked();
    system.useHighDpiPixmaps = d->useHighDpiPixmapsCheck->isChecked();

#endif

    system.softwareOpenGL    = d->softwareOpenGLCheck->isChecked();
    system.enableLogging     = d->enableLoggingCheck->isChecked();
    system.enableOpenCL      = d->enableOpenCLCheck->isChecked();
    system.enableDnnOpenCL   = d->enableOpenCLDNNCheck->isChecked() &&
                               d->enableOpenCLCheck->isChecked()    &&
                               d->openCLDNNTestResult;
    system.dnnOpenCLTested   = d->openCLDNNTestResult;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    system.enableHWVideo     = d->enableHWVideoCheck->isChecked();
    system.enableHWTConv     = d->enableHWTConvCheck->isChecked();
    system.videoBackend      = d->videoBackendCBox->currentData().toString();

#endif

    system.proxyUrl          = d->uiProxySettings.kcfg_proxyUrl->text();
    system.proxyPort         = d->uiProxySettings.kcfg_proxyPort->value();
    system.proxyType         = d->uiProxySettings.kcfg_proxyType->currentIndex();

    if (d->uiProxySettings.kcfg_proxyAuth->isChecked())
    {
        system.proxyAuth = true;
        system.proxyUser = d->uiProxySettings.kcfg_proxyUser->text();
        system.proxyPass = d->uiProxySettings.kcfg_proxyPass->text();
    }
    else
    {
        system.proxyAuth = false;
    }

    system.saveSettings();
    system.applyProxySettings();
}

void SystemSettingsWidget::slotBinaryDownload()
{
    QPointer<FilesDownloader> floader = new FilesDownloader(this);
    floader->startDownload();
    delete floader;
}

void SystemSettingsWidget::slotOpenCLDNNTest()
{
    d->openCLDNNTest->setEnabled(false);

    d->openCLDNNTestDlg = new OpenCVOpenCLDNNTestDlg(this);
    d->openCLDNNTestDlg->setAttribute(Qt::WA_DeleteOnClose);
    d->openCLDNNTestDlg->setWindowModality(Qt::WindowModal);

    // Connect to the finished signal to get the test result

    connect(d->openCLDNNTestDlg, &QDialog::finished, 
            this, &SystemSettingsWidget::slotOpenCLDNNTestFinished);    

    // display the test dialog

    d->openCLDNNTestDlg->show();
}

void SystemSettingsWidget::slotOpenCLDNNTestFinished()
{
    // Re-enable the test button

    d->openCLDNNTest->setEnabled(true);

    // Process the dialog result

    d->openCLDNNTestResult = (d->openCLDNNTestDlg->result() == QDialog::Accepted);

    d->enableOpenCLDNNCheck->setEnabled(d->enableOpenCLCheck->isChecked() && d->openCLDNNTestResult);
    d->enableOpenCLDNNCheck->setChecked(d->enableOpenCLCheck->isChecked() && d->openCLDNNTestResult);
}

void SystemSettingsWidget::slotOpenLocalStorage()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(FilesDownloader::getFacesEnginePath()));
}

} // namespace Digikam

#include "moc_systemsettingswidget.cpp"
