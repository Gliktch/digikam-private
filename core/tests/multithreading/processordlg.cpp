/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : test for implementation of thread manager api
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "processordlg.h"

// Qt includes

#include <QCoreApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QThreadPool>
#include <QFileInfo>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSpinBox>

// Local includes

#include "digikam_debug.h"
#include "rawtopngconverterthread.h"

class Q_DECL_HIDDEN ProcessorDlg::Private
{
public:

    explicit Private()
      : count        (0),
        page         (nullptr),
        items        (nullptr),
        buttons      (nullptr),
        progressView (nullptr),
        usedCore     (nullptr),
        thread       (nullptr)
    {
    }

public:

    int                      count;

    QWidget*                 page;
    QLabel*                  items;
    QDialogButtonBox*        buttons;
    QScrollArea*             progressView;

    QList<QUrl>              list;

    QSpinBox*                usedCore;
    RAWToPNGConverterThread* thread;
};

ProcessorDlg::ProcessorDlg(const QList<QUrl>& list, QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setModal(false);
    setWindowTitle(QString::fromUtf8("Convert RAW Files to PNG"));

    d->buttons               = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close, this);
    d->thread                = new RAWToPNGConverterThread(this);
    d->list                  = list;
    d->count                 = d->list.count();
    qCDebug(DIGIKAM_TESTS_LOG) << d->list;

    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    int cpu                  = d->thread->maximumNumberOfThreads();
    QGridLayout* const grid  = new QGridLayout(d->page);
    QLabel* const pid        = new QLabel(QString::fromUtf8("Main PID: %1").arg(QCoreApplication::applicationPid()),  this);
    QLabel* const core       = new QLabel(QString::fromUtf8("CPU cores available: %1").arg(cpu), this);
    QWidget* const hbox      = new QWidget(this);
    d->items                 = new QLabel(this);

    QHBoxLayout* const hlay  = new QHBoxLayout(hbox);
    QLabel* const coresLabel = new QLabel(QString::fromUtf8("Cores to use: "), this);
    d->usedCore              = new QSpinBox(this);
    d->usedCore->setRange(1, cpu);
    d->usedCore->setSingleStep(1);
    d->usedCore->setValue(cpu);
    hlay->addWidget(coresLabel);
    hlay->addWidget(d->usedCore);
    hlay->setContentsMargins(QMargins());

    d->progressView = new QScrollArea(this);
    QWidget* const progressbox      = new QWidget(d->progressView->viewport());
    QVBoxLayout* const progressLay  = new QVBoxLayout(progressbox);
    d->progressView->setWidget(progressbox);
    d->progressView->setWidgetResizable(true);

    grid->addWidget(pid,             0, 0, 1, 1);
    grid->addWidget(core,            1, 0, 1, 1);
    grid->addWidget(hbox,            2, 0, 1, 1);
    grid->addWidget(d->items,        3, 0, 1, 1);
    grid->addWidget(d->progressView, 4, 0, 1, 1);

    for (const QUrl& url : std::as_const(d->list))
    {
        QProgressBar* const bar = new QProgressBar(progressbox);
        QString file            = url.toLocalFile();
        QFileInfo fi(file);
        bar->setMaximum(100);
        bar->setMinimum(0);
        bar->setValue(100);
        bar->setObjectName(file);
        bar->setFormat(fi.fileName());
        progressLay->addWidget(bar);
    }

    progressLay->addStretch();

    QPushButton* const applyBtn  = d->buttons->button(QDialogButtonBox::Apply);
    QPushButton* const cancelBtn = d->buttons->button(QDialogButtonBox::Close);

    connect(applyBtn, SIGNAL(clicked()),
            this, SLOT(slotStart()));

    connect(cancelBtn, SIGNAL(clicked()),
            this, SLOT(slotStop()));

    connect(d->thread, SIGNAL(signalStarting(QUrl)),
            this, SLOT(slotStarting(QUrl)));

    connect(d->thread, SIGNAL(signalFinished(QUrl)),
            this, SLOT(slotFinished(QUrl)));

    connect(d->thread, SIGNAL(signalFailed(QUrl,QString)),
            this, SLOT(slotFailed(QUrl,QString)));

    connect(d->thread, SIGNAL(signalProgress(QUrl,int)),
            this, SLOT(slotProgress(QUrl,int)));

    updateCount();
    resize(500, 400);
}

ProcessorDlg::~ProcessorDlg()
{
    delete d;
}

void ProcessorDlg::updateCount()
{
    d->items->setText(QString::fromUtf8("Files to process : %1").arg(d->count));
}

void ProcessorDlg::slotStart()
{
    if (d->list.isEmpty()) return;

    d->buttons->button(QDialogButtonBox::Apply)->setDisabled(true);
    d->usedCore->setDisabled(true);

    d->thread->setMaximumNumberOfThreads(d->usedCore->value());
    d->thread->convertRAWtoPNG(d->list, DRawDecoderSettings());
    d->thread->start();
}

void ProcessorDlg::slotStop()
{
    d->thread->cancel();
    reject();
}

QProgressBar* ProcessorDlg::findProgressBar(const QUrl& url) const
{
    QList<QProgressBar*> bars = findChildren<QProgressBar*>();

    for (QProgressBar* const b : std::as_const(bars))
    {
        if (b->objectName() == url.toLocalFile())
        {   // cppcheck-suppress useStlAlgorithm
            return b;
        }
    }

    qCWarning(DIGIKAM_TESTS_LOG) << "Cannot found relevant progress bar for " << url.toLocalFile();

    return nullptr;
}

void ProcessorDlg::slotStarting(const QUrl& url)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start to process item " << url.toLocalFile();

    QProgressBar* const b = findProgressBar(url);

    if (b)
    {
        d->progressView->ensureWidgetVisible(b);
        b->setMinimum(0);
        b->setMaximum(100);
        b->setValue(0);
    }
}

void ProcessorDlg::slotProgress(const QUrl& url,int p)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Processing item " << url.toLocalFile() << " : " << p << " %";;

    QProgressBar* const b = findProgressBar(url);

    if (b)
    {
        b->setMinimum(0);
        b->setMaximum(100);
        b->setValue(p);
    }
}

void ProcessorDlg::slotFinished(const QUrl& url)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Completed item " << url.toLocalFile();

    QProgressBar* const b = findProgressBar(url);

    if (b)
    {
        b->setMinimum(0);
        b->setMaximum(100);
        b->setValue(100);
        b->setFormat(QString::fromUtf8("Done"));
        d->count--;
        updateCount();
    }
}

void ProcessorDlg::slotFailed(const QUrl& url, const QString& err)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Failed to complete item " << url.toLocalFile();

    QProgressBar* const b = findProgressBar(url);

    if (b)
    {
        b->setMinimum(0);
        b->setMaximum(100);
        b->setValue(100);
        b->setFormat(err);
        d->count--;
        updateCount();
    }
}

#include "moc_processordlg.cpp"
