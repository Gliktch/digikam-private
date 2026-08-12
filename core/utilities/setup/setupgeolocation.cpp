/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-12-24
 * Description : geo-location setup page.
 *
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupgeolocation.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QStandardPaths>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QList>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "MarbleConfigView.h"
#include "geolocationsettings.h"
#include "digikam_debug.h"

using namespace Marble;

namespace Digikam
{

class Q_DECL_HIDDEN SetupGeolocation::Private
{
public:

    Private() = default;

public:

    MarbleConfigView* tab      = nullptr;
    QWidget*          gMapView = nullptr;
    QLineEdit*        gMapApi  = nullptr;

    QString           oldKey;

public:

    QString htmlFilePath(QString& htmlPath) const
    {
        htmlPath  = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        htmlPath += QLatin1String("/digikam/geoiface/");

        return (htmlPath + QLatin1String("backend-googlemaps.html"));
    }

    QString htmlTemplate() const
    {
        return QLatin1String("<html>\n<head>\n"
                             "<base href=\"%1\"/>\n"
                             "<script type=\"text/javascript\" src=\"https://maps.google.com/maps/"
                             "api/js?key=%2\"></script>\n"
                             "<script type=\"text/javascript\" src=\"backend-googlemaps-js.js\"></script>\n"
                             "</head>\n"
                             "<body onload=\"kgeomapInitialize()\" style=\"padding: 0px; margin: 0px;\">\n"
                             "    <div id=\"map_canvas\" style=\"width:100%; height:400px;\"></div>\n"
                             "</body>\n</html>\n");
    }
};

SetupGeolocation::SetupGeolocation(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    QWidget* const panel     = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    const int spacing       = layoutSpacing();


    QVBoxLayout* const vbl  = new QVBoxLayout();
    panel->setLayout(vbl);

    d->tab                  = new MarbleConfigView(GeolocationSettings::instance()->mainMarbleWidget(), this);

    // ---

    d->gMapView             = new QWidget(this);
    QLabel* const title     = new QLabel(i18n("API Key:"), d->gMapView);
    d->gMapApi              = new QLineEdit(d->gMapView);
    d->gMapApi->setEchoMode(QLineEdit::Normal);

    QFrame* const infoBox       = new QFrame;
    QVBoxLayout* const infoVlay = new QVBoxLayout;
    infoBox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QLabel* const explanation   = new QLabel(infoBox);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    QString txt = i18nc("@info", "To work friendly, Google Maps requires an API key that end users must buy online "
                                 "to gain credentials. Without this key, Google Maps is still usable but with an "
                                 "overlay frame annotated \"For Development Only\". For more information, see this "
                                 "<a href='https://mapsplatform.google.com/pricing/'>Google page</a> about "
                                 "usages, practices, and prices.");

    explanation->setText(txt);
    infoVlay->addWidget(explanation);
    infoVlay->setContentsMargins(spacing, spacing, spacing, spacing);
    infoVlay->setSpacing(0);
    infoBox->setLayout(infoVlay);

    QVBoxLayout* const vlay = new QVBoxLayout(d->gMapView);
    vlay->addWidget(title);
    vlay->addWidget(d->gMapApi);
    vlay->addWidget(infoBox);
    vlay->addStretch();
    d->tab->insertTab(GoogleMaps, d->gMapView, i18n("Google Maps"));

    // ---

    vbl->addWidget(d->tab);

    readSettings();
}

SetupGeolocation::~SetupGeolocation()
{
    delete d;
}

void SetupGeolocation::cancel()
{
    d->tab->cancel();
}

void SetupGeolocation::setActiveTab(GeolocationTab tab)
{
    d->tab->setCurrentIndex(tab);
}

SetupGeolocation::GeolocationTab SetupGeolocation::activeTab() const
{
    return (GeolocationTab)d->tab->currentIndex();
}

void SetupGeolocation::applySettings()
{
    d->tab->applySettings();

    if (d->gMapApi->text() == d->oldKey)
    {
        return;
    }

    QString htmlPath;
    QString key      = d->gMapApi->text().trimmed();
    QString htmlFile = d->htmlFilePath(htmlPath);

    if (key.isEmpty())
    {
        if (QFileInfo::exists(htmlFile))
        {
            QFile::remove(htmlFile);
        }

        GeolocationSettings::instance()->reloadGoogleMaps();
        return;
    }

    QString jsFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                            QLatin1String("digikam/geoiface/backend-googlemaps-js.js"));

    if (!QFileInfo::exists(jsFile))
    {
        return;
    }

    QUrl baseUrl     = QUrl::fromLocalFile(jsFile).adjusted(QUrl::RemoveFilename);
    QString htmlText = d->htmlTemplate().arg(baseUrl.toString()).arg(key);

    if (!QFileInfo::exists(htmlPath))
    {
        QDir().mkpath(htmlPath);
    }

    QFile writeFile(htmlFile);

    if (writeFile.open(QIODevice::WriteOnly))
    {
        writeFile.write(htmlText.toUtf8());
        writeFile.close();

        GeolocationSettings::instance()->reloadGoogleMaps();
    }
}

void SetupGeolocation::readSettings()
{
    d->tab->readSettings();

    // Read Google API key

    QString htmlPath;
    QString htmlFile = d->htmlFilePath(htmlPath);

    if (QFileInfo::exists(htmlFile))
    {
        QFile readFile(htmlFile);

        if (readFile.open(QIODevice::ReadOnly))
        {
            QString oldHtml = QString::fromUtf8(readFile.readAll());
            readFile.close();

            if (!oldHtml.isEmpty())
            {
                int firstIdx = oldHtml.indexOf(QLatin1String("key="));

                if (firstIdx != -1)
                {
                    int lastIdx = oldHtml.indexOf(QLatin1String("\"></script>"), firstIdx);

                    if (lastIdx > (firstIdx + 4))
                    {
                        d->oldKey = oldHtml.mid(firstIdx + 4, lastIdx - firstIdx - 4);
                    }
                }
            }
        }
    }

    d->gMapApi->setText(d->oldKey);
}

} // namespace Digikam

#include "moc_setupgeolocation.cpp"
