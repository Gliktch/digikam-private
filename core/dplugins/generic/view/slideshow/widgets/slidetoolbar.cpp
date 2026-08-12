/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-10-05
 * Description : a tool bar for slideshow
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 * SPDX-FileCopyrightText:      2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slidetoolbar.h"

// Qt includes

#include <QApplication>
#include <QActionGroup>
#include <QToolButton>
#include <QPixmap>
#include <QScreen>
#include <QMenu>
#include <QEventLoop>
#include <QInputDialog>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dpluginaboutdlg.h"
#include "digikam_debug.h"
#include "setupslideshow_dialog.h"

namespace DigikamGenericSlideShowPlugin
{

class Q_DECL_HIDDEN SlideToolBar::Private
{
public:

    Private() = default;

public:

    QToolButton*          playBtn           = nullptr;
    QToolButton*          prevBtn           = nullptr;
    QToolButton*          nextBtn           = nullptr;
    QToolButton*          stopBtn           = nullptr;
    QToolButton*          delayBtn          = nullptr;
    QToolButton*          deleteBtn         = nullptr;
    QToolButton*          setupBtn          = nullptr;
    QToolButton*          screenSelectBtn   = nullptr;

    QWidget*              parentWidget      = nullptr;

    bool                  currentlyPause    = false;

    SlideShowSettings*    settings          = nullptr;
};

SlideToolBar::SlideToolBar(SlideShowSettings* const settings, QWidget* const parent)
    : DHBox(parent),
      d    (new Private)
{
    setMouseTracking(true);
    setContentsMargins(QMargins());

    d->settings       = settings;
    d->parentWidget   = parent;

    d->playBtn        = new QToolButton(this);
    d->prevBtn        = new QToolButton(this);
    d->nextBtn        = new QToolButton(this);
    d->stopBtn        = new QToolButton(this);
    d->delayBtn       = new QToolButton(this);
    d->deleteBtn      = new QToolButton(this);
    d->setupBtn       = new QToolButton(this);

    d->playBtn->setCheckable(true);
    d->playBtn->setChecked(!d->settings->autoPlayEnabled);
    d->playBtn->setFocusPolicy(Qt::NoFocus);
    d->prevBtn->setFocusPolicy(Qt::NoFocus);
    d->nextBtn->setFocusPolicy(Qt::NoFocus);
    d->stopBtn->setFocusPolicy(Qt::NoFocus);
    d->delayBtn->setFocusPolicy(Qt::NoFocus);
    d->deleteBtn->setFocusPolicy(Qt::NoFocus);
    d->setupBtn->setFocusPolicy(Qt::NoFocus);

    QSize s(32, 32);
    d->playBtn->setIconSize(s);
    d->prevBtn->setIconSize(s);
    d->nextBtn->setIconSize(s);
    d->stopBtn->setIconSize(s);
    d->delayBtn->setIconSize(s);
    d->deleteBtn->setIconSize(s);
    d->setupBtn->setIconSize(s);

    QString iconString = d->settings->autoPlayEnabled ? QLatin1String("media-playback-pause")
                                                      : QLatin1String("media-playback-start");
    d->playBtn->setIcon(QIcon::fromTheme(iconString));
    d->prevBtn->setIcon(QIcon::fromTheme(QLatin1String("media-skip-backward")));
    d->nextBtn->setIcon(QIcon::fromTheme(QLatin1String("media-skip-forward")));
    d->stopBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-stop")));
    d->delayBtn->setIcon(QIcon::fromTheme(QLatin1String("appointment-new")));
    d->deleteBtn->setIcon(QIcon::fromTheme(QLatin1String("user-trash")));
    d->setupBtn->setIcon(QIcon::fromTheme(QLatin1String("systemsettings")));

    int num = qApp->screens().count();

    if (num > 1)
    {
        d->screenSelectBtn        = new QToolButton(this);
        QMenu* const screenMenu   = new QMenu(d->screenSelectBtn);
        d->screenSelectBtn->setToolTip(i18n("Switch Screen"));
        d->screenSelectBtn->setIconSize(s);
        d->screenSelectBtn->setIcon(QIcon::fromTheme(QLatin1String("video-display")));
        d->screenSelectBtn->setMenu(screenMenu);
        d->screenSelectBtn->setPopupMode(QToolButton::InstantPopup);
        d->screenSelectBtn->setFocusPolicy(Qt::NoFocus);

        QActionGroup* const group = new QActionGroup(screenMenu);
        group->setExclusive(true);

        for (int i = 0 ; i < num ; ++i)
        {
            QString model      = qApp->screens().at(i)->model();
            QAction* const act = screenMenu->addAction(i18nc("%1 is the screen number (0, 1, ...)", "Screen %1", i) +
                                                       QString::fromUtf8(" (%1)").arg(model.left(model.length() - 1)));
            act->setData(QVariant::fromValue(i));
            act->setCheckable(true);
            group->addAction(act);

            if (i == d->settings->slideScreen)
            {
               act->setChecked(true);
            }
        }

        connect(screenMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotScreenSelected(QAction*)));
    }

    connect(d->playBtn, SIGNAL(toggled(bool)),
            this, SLOT(slotPlayBtnToggled()));

    connect(d->nextBtn, SIGNAL(clicked()),
            this, SLOT(slotNexPrevClicked()));

    connect(d->prevBtn, SIGNAL(clicked()),
            this, SLOT(slotNexPrevClicked()));

    connect(d->nextBtn, SIGNAL(clicked()),
            this, SIGNAL(signalNext()));

    connect(d->prevBtn, SIGNAL(clicked()),
            this, SIGNAL(signalPrev()));

    connect(d->stopBtn, SIGNAL(clicked()),
            this, SIGNAL(signalClose()));

    connect(d->delayBtn, SIGNAL(clicked()),
            this, SLOT(slotChangeDelayButtonPressed()));

    connect(d->deleteBtn, SIGNAL(clicked()),
            this, SLOT(slotRemoveImage()));

    connect(d->setupBtn, SIGNAL(clicked()),
            this, SLOT(slotMenuSlideShowConfiguration()));
}

SlideToolBar::~SlideToolBar()
{
    delete d;
}

bool SlideToolBar::isPaused() const
{
    return d->playBtn->isChecked();
}

void SlideToolBar::pause(bool val)
{
    if (val == isPaused())
    {
        return;
    }

    d->playBtn->setChecked(val);
    slotPlayBtnToggled();
}

void SlideToolBar::setEnabledPlay(bool val)
{
    d->playBtn->setEnabled(val);
}

void SlideToolBar::setEnabledNext(bool val)
{
    d->nextBtn->setEnabled(val);
}

void SlideToolBar::setEnabledPrev(bool val)
{
    d->prevBtn->setEnabled(val);
}

void SlideToolBar::setEnabledDelete(bool val)
{
    d->deleteBtn->setEnabled(val);
}

void SlideToolBar::slotPlayBtnToggled()
{
    if (d->playBtn->isChecked())
    {
        d->playBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));

        Q_EMIT signalPause();
    }
    else
    {
        d->playBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));

        Q_EMIT signalPlay();
    }
}

void SlideToolBar::slotChangeDelayButtonPressed()
{
    bool ok;
    int delay    = d->settings->delay;
    bool running = (!isPaused() && d->playBtn->isEnabled());

    if (running)
    {
        d->playBtn->animateClick();
    }

    delay = QInputDialog::getInt(this, i18nc("@title:window", "Specify Delay for Slide Show"),
                                       i18n("Delay:"), delay, 1, 3600, 1, &ok);

    d->parentWidget->setFocus();

    if (ok)
    {
        d->settings->delay = delay;
    }

    if (running)
    {
        d->playBtn->animateClick();
    }
}

void SlideToolBar::slotNexPrevClicked()
{
    if (!d->playBtn->isChecked())
    {
        d->playBtn->setChecked(true);
        d->playBtn->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));

        Q_EMIT signalPause();
    }
}

void SlideToolBar::slotMenuSlideShowConfiguration()
{
    d->currentlyPause                    = isPaused();

    if (!d->currentlyPause && d->playBtn->isEnabled())
    {
        d->playBtn->animateClick();
    }

    QPointer<SetupSlideShowDialog> setup = new SetupSlideShowDialog(d->settings);
    int ret                              = QDialog::Rejected;
    setup->show();

    QEventLoop loop;

    connect(setup, &QDialog::finished,
            this, [&loop, setup, &ret](int result)
        {
            ret = result;
            loop.quit();
            delete setup;
        }
    );

    loop.exec();

    d->parentWidget->setFocus();

    if (ret == QDialog::Accepted)
    {
        Q_EMIT signalUpdateSettings();
    }

    if (!d->currentlyPause && d->playBtn->isEnabled())
    {
        d->playBtn->animateClick();
    }
}

void SlideToolBar::keyPressEvent(QKeyEvent* e)
{
    if      (e->key() == Qt::Key_F1)
    {
        d->currentlyPause              = isPaused();

        if (!d->currentlyPause && d->playBtn->isEnabled())
        {
            d->playBtn->animateClick();
        }

        QPointer<DPluginAboutDlg> help = new DPluginAboutDlg(d->settings->plugin);
        help->show();

        QEventLoop loop;

        connect(help, &QDialog::finished,
                this, [&loop, help]()
            {
                loop.quit();
                delete help;
            }
        );

        loop.exec();

        d->parentWidget->setFocus();

        if (!d->currentlyPause && d->playBtn->isEnabled())
        {
            d->playBtn->animateClick();
        }
    }
    else if (
             (e->key()       == Qt::Key_P) &&
             (e->modifiers() == Qt::AltModifier)
            )
    {
        slotMenuSlideShowConfiguration();
    }
    else if (e->key() == Qt::Key_Space)
    {
        if (d->playBtn->isEnabled())
        {
            d->playBtn->animateClick();
        }
    }
    else if (
             (e->key() == Qt::Key_Left)     ||
             (e->key() == Qt::Key_Up)       ||
             (e->key() == Qt::Key_PageUp)
            )
    {
        if (d->prevBtn->isEnabled())
        {
            d->prevBtn->animateClick();
        }
    }
    else if (
             (e->key() == Qt::Key_Right)     ||
             (e->key() == Qt::Key_Down)      ||
             (e->key() == Qt::Key_PageDown)
            )
    {
        if (d->nextBtn->isEnabled())
        {
            d->nextBtn->animateClick();
        }
    }
    else if ( (e->key() == Qt::Key_Delete) &&
              (e->modifiers() == Qt::NoModifier) )
    {
        if (d->deleteBtn->isEnabled())
        {
            d->deleteBtn->animateClick();
        }
    }
    else if (e->key() == Qt::Key_Escape)
    {
        if (d->stopBtn->isEnabled())
        {
            d->stopBtn->animateClick();
        }
    }
    else
    {
        qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Key not captured by Slideshow toolbar:"
                                               << e->key() << e->modifiers();
    }

    e->accept();
}

void SlideToolBar::slotScreenSelected(QAction* act)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (!act || (act->data().typeId() != QMetaType::Int))

#else

    if (!act || (act->data().type() != QVariant::Int))

#endif

    {
        return;
    }

    Q_EMIT signalScreenSelected(act->data().toInt());
}

void SlideToolBar::slotRemoveImage()
{
    bool running = (!isPaused() && d->playBtn->isEnabled());

    if (running)
    {
        d->playBtn->animateClick();
    }

    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Question,
             i18nc("@title:window", "Delete Image"),
             i18n("Do you want to move this image to the trash?"),
             QMessageBox::Yes | QMessageBox::No, this);

    msgBox->setDefaultButton(QMessageBox::Yes);

    int ret = msgBox->exec();
    delete msgBox;

    d->parentWidget->setFocus();

    if (ret == QMessageBox::Yes)
    {
        Q_EMIT signalRemoveImageFromList();
    }

    if (running)
    {
        d->playBtn->animateClick();
    }
}

} // namespace DigikamGenericSlideShowPlugin

#include "moc_slidetoolbar.cpp"
