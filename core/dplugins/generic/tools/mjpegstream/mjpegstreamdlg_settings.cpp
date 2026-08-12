/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : MJPEG Stream configuration dialog - Settings methods
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mjpegstreamdlg_p.h"

namespace DigikamGenericMjpegStreamPlugin
{

void MjpegStreamDlg::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->mngr->configGroupName());

    d->startOnStartup->setChecked(group.readEntry(d->mngr->configStartServerOnStartupEntry(), false));
    d->settings.readSettings(group);

    d->srvPort->blockSignals(true);
    d->delay->blockSignals(true);
    d->rate->blockSignals(true);
    d->quality->blockSignals(true);
    d->streamLoop->blockSignals(true);
    d->typeVal->blockSignals(true);
    d->effVal->blockSignals(true);
    d->transVal->blockSignals(true);

    d->srvPort->setValue(d->settings.port);
    d->delay->setValue(d->settings.delay);
    d->rate->setValue(d->settings.rate);
    d->quality->setValue(d->settings.quality);
    d->streamLoop->setChecked(d->settings.loop);
    d->typeVal->setCurrentIndex(d->settings.outSize);
    d->effVal->setCurrentIndex(d->settings.effect);
    d->transVal->setCurrentIndex(d->settings.transition);

    d->frameOsdWidget->setSettings(d->settings.osdSettings);

    d->srvPort->blockSignals(false);
    d->delay->blockSignals(false);
    d->rate->blockSignals(false);
    d->quality->blockSignals(false);
    d->streamLoop->blockSignals(false);
    d->typeVal->blockSignals(false);
    d->effVal->blockSignals(false);
    d->transVal->blockSignals(false);

    slotSettingsChanged();

    updateServerStatus();
}

void MjpegStreamDlg::saveSettings()
{
    setMjpegServerContents();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->mngr->configGroupName());
    group.writeEntry(d->mngr->configStartServerOnStartupEntry(), d->startOnStartup->isChecked());
    d->settings.writeSettings(group);
    config->sync();
}

void MjpegStreamDlg::slotSettingsChanged()
{
    d->settings.port                              = d->srvPort->value();
    d->settings.delay                             = d->delay->value();
    d->settings.rate                              = d->rate->value();
    d->settings.quality                           = d->quality->value();
    d->settings.loop                              = d->streamLoop->isChecked();
    d->settings.outSize                           = d->typeVal->currentIndex();
    d->settings.effect                            = (EffectMngr::EffectType)d->effVal->currentIndex();
    d->settings.transition                        = (TransitionMngr::TransType)d->transVal->currentIndex();

    d->settings.osdSettings                       = d->frameOsdWidget->settings();

    d->effPreview->stopPreview();
    d->effPreview->startPreview(d->settings.effect);
    d->transPreview->stopPreview();
    d->transPreview->startPreview(d->settings.transition);
}

} // namespace DigikamGenericMjpegStreamPlugin
