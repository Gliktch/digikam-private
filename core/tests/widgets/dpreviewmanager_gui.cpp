/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for implementation of DPreviewManager api
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpreviewmanager_gui.h"

// Qt includes

#include <QGridLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QUrl>
#include <QLabel>
#include <QPushButton>

// Local includes

#include "digikam_debug.h"
#include "metaengine.h"
#include "dpluginloader.h"
#include "dpreviewmanager.h"
#include "dfiledialog.h"

using namespace Digikam;

class Q_DECL_HIDDEN DPreviewManagerTest::Private
{
public:

    Private() = default;

public:

    QWidget*          page     = nullptr;

    QDialogButtonBox* buttons  = nullptr;
    QLabel*           label    = nullptr;
    QPushButton*      loadBtn  = nullptr;

    DPreviewManager*  preview  = nullptr;

    QString           lastFileOpenPath;
};

DPreviewManagerTest::DPreviewManagerTest(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setWindowTitle(QString::fromUtf8("Select an Area Over the Image"));

    d->buttons               = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close, this);
    d->loadBtn = d->buttons->button(QDialogButtonBox::Apply);
    d->loadBtn->setText(QLatin1String("Load Image..."));

    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
    setModal(false);

    // -------------------

    QGridLayout* const mainLayout = new QGridLayout(d->page);

    d->preview                    = new DPreviewManager(d->page);
    d->preview->setButtonVisible(false);
    d->preview->setSelectionAreaPossible(true);

    d->label                      = new QLabel(d->page);

    mainLayout->addWidget(d->preview,    0, 0, 1, 1);
    mainLayout->addWidget(d->label,      1, 0, 1, 1);
    mainLayout->setRowStretch(0, 10);

    connect(d->loadBtn, &QPushButton::clicked,
            this, &DPreviewManagerTest::slotLoadImage);

    connect(d->buttons->button(QDialogButtonBox::Close), &QPushButton::clicked,
            this, &DPreviewManagerTest::close);

    connect(d->preview, SIGNAL(signalSelectionChanged(QRectF)),
            this, SLOT(slotSelectionChanged(QRectF)));
}

DPreviewManagerTest::~DPreviewManagerTest()
{
    delete d;
}

void DPreviewManagerTest::slotLoadImage()
{
    QString file = DFileDialog::getOpenFileName(this, QLatin1String("Select Image to Open"),
                                                d->lastFileOpenPath,
                                                QString::fromLatin1("Image Files (*.png *.jpg *.bmp *.pgm)"));

    if (file.isEmpty())
    {
        return;
    }

    d->lastFileOpenPath = QFileInfo(file).absolutePath();

    d->preview->load(QUrl::fromLocalFile(file));
}

void DPreviewManagerTest::slotSelectionChanged(const QRectF& sel)
{
    d->label->setText(QString::fromLatin1("Selection: (%1, %2) [%3, %4]")
                      .arg(sel.left()).arg(sel.top()).arg(sel.width()).arg(sel.height()));
}

int main(int argc, char* argv[])
{
    MetaEngine::initializeExiv2();

    QDir dir(qApp->applicationDirPath());
    qputenv("DK_PLUGIN_PATH", dir.canonicalPath().toUtf8());
    DPluginLoader::instance()->init();

    QApplication app(argc, argv);
    DPreviewManagerTest* const view = new DPreviewManagerTest;
    view->show();
    app.exec();
    delete view;

    DPluginLoader::instance()->cleanUp();

    return 0;
}
