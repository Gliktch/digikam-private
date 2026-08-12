/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchwindow.h"

// Qt includes

#include <QApplication>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScreen>
#include <QWindow>


// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "searchview.h"
#include "coredbsearchxml.h"
#include "thememanager.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN SearchWindow::Private
{
public:

    Private() = default;

    QScrollArea*         scrollArea     = nullptr;
    SearchView*          searchView     = nullptr;
    SearchViewBottomBar* bottomBar      = nullptr;
    int                  currentId      = -1;
    bool                 hasTouchedXml  = false;
    QString              oldXml;
};

SearchWindow::SearchWindow()
    : QWidget(nullptr),
      d      (new Private)
{
    QVBoxLayout* const layout = new QVBoxLayout;

    d->scrollArea             = new QScrollArea(this);
    d->scrollArea->setWidgetResizable(true);
    d->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->searchView             = new SearchView;
    d->searchView->setup();

    d->bottomBar              = new SearchViewBottomBar(d->searchView);
    d->searchView->setBottomBar(d->bottomBar);

    d->scrollArea->setWidget(d->searchView);
    d->scrollArea->setFrameStyle(QFrame::NoFrame);

    layout->addWidget(d->scrollArea);
    layout->addWidget(d->bottomBar);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    setLayout(layout);

    setVisible(false);
    setWindowTitle(i18nc("@title:window", "Advanced Search"));

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = qApp->activeWindow())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QRect srect = screen->availableGeometry();
    QSize wsize = QSize(1024 <= srect.width()  ? 1024 : srect.width(),
                         800 <= srect.height() ?  800 : srect.height());

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("AdvancedSearch Widget"));

    if (group.exists())
    {
        QSize confSize = group.readEntry(QLatin1String("Widget Size"), wsize);
        resize(confSize);
    }
    else
    {
        resize(wsize);
    }

    connect(d->searchView, SIGNAL(searchOk()),
            this, SLOT(searchOk()));

    connect(d->searchView, SIGNAL(searchCancel()),
            this, SLOT(searchCancel()));

    connect(d->searchView, SIGNAL(searchTryout()),
            this, SLOT(searchTryout()));
}

SearchWindow::~SearchWindow()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("AdvancedSearch Widget"));
    group.writeEntry(QLatin1String("Widget Size"), size());

    delete d;
}

void SearchWindow::readSearch(int id, const QString& xml)
{
    d->currentId     = id;
    d->hasTouchedXml = false;
    d->oldXml        = xml;
    d->searchView->read(xml);
}

void SearchWindow::reset()
{
    d->currentId     = -1;
    d->hasTouchedXml = false;
    d->oldXml.clear();
    d->searchView->read(QString());
}

QString SearchWindow::search() const
{
    return d->searchView->write();
}

void SearchWindow::searchOk()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "SearchWindow: search confirmed";

    d->hasTouchedXml = true;

    Q_EMIT searchEdited(d->currentId, search());

    hide();
}

void SearchWindow::searchCancel()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "SearchWindow: search cancelled";

    // redo changes by tryout

    if (d->hasTouchedXml)
    {
        Q_EMIT searchEdited(d->currentId, d->oldXml);

        d->hasTouchedXml = false;
    }

    hide();
}

void SearchWindow::searchTryout()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "SearchWindow: search tryout";

    d->hasTouchedXml = true;

    Q_EMIT searchEdited(d->currentId, search());
}

void SearchWindow::keyPressEvent(QKeyEvent* e)
{
    // Implement keys like in a dialog

    if (!e->modifiers() || ((e->modifiers() & Qt::KeypadModifier) && (e->key() == Qt::Key_Enter)))
    {
        switch (e->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Select:
            {
                searchOk();
                break;
            }

            case Qt::Key_F4:
            case Qt::Key_Escape:
            case Qt::Key_Back:
            {
                searchCancel();
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

} // namespace Digikam

#include "moc_searchwindow.cpp"
