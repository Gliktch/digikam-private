/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-01
 * Description : a tool to export images to Smugmug web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "dinfointerface.h"
#include "dprogresswdg.h"
#include "ditemslist.h"

class QLabel;
class QSpinBox;
class QCheckBox;
class QRadioButton;
class QPushButton;
class QComboBox;
class QLineEdit;

using namespace Digikam;

namespace DigikamGenericSmugPlugin
{

class SmugWidget : public QWidget
{
    Q_OBJECT

public:

    explicit SmugWidget(QWidget* const parent,
                        DInfoInterface* const iface,
                        bool import);
    ~SmugWidget() override = default;

public:

    void updateLabels(const QString& email = QString(),
                      const QString& name = QString(),
                      const QString& nick = QString());

    bool isAnonymous()           const;
    void setAnonymous(bool checked);

    QString getNickName()        const;
    void    setNickName(const QString& nick);

    QString getSitePassword()    const;
    QString getAlbumPassword()   const;
    QString getDestinationPath() const;

    DItemsList* imagesList()     const;
    DProgressWdg* progressBar()  const;

Q_SIGNALS:

    void signalUserChangeRequest(bool anonymous);

private Q_SLOTS:

    void slotAnonymousToggled(bool checked);
    void slotChangeUserClicked();
    void slotResizeChecked();

private:

    QLabel*         m_headerLbl         = nullptr;
    QLabel*         m_userNameLbl       = nullptr;
    QLabel*         m_userName          = nullptr;
    QLabel*         m_emailLbl          = nullptr;
    QLabel*         m_email             = nullptr;
    QLabel*         m_nickNameLbl       = nullptr;
    QLabel*         m_sitePasswordLbl   = nullptr;
    QLabel*         m_albumPasswordLbl  = nullptr;

    QRadioButton*   m_anonymousRBtn     = nullptr;
    QRadioButton*   m_accountRBtn       = nullptr;

    QCheckBox*      m_resizeChB         = nullptr;

    QSpinBox*       m_dimensionSpB      = nullptr;
    QSpinBox*       m_imageQualitySpB   = nullptr;

    QComboBox*      m_albumsCoB         = nullptr;

    QPushButton*    m_newAlbumBtn       = nullptr;
    QPushButton*    m_reloadAlbumsBtn   = nullptr;
    QPushButton*    m_changeUserBtn     = nullptr;

    QLineEdit*      m_albumPasswordEdt  = nullptr;
    QLineEdit*      m_nickNameEdt       = nullptr;
    QLineEdit*      m_sitePasswordEdt   = nullptr;

    DInfoInterface* m_iface             = nullptr;
    DProgressWdg*   m_progressBar       = nullptr;
    DItemsList*     m_imgList           = nullptr;
    QWidget*        m_uploadWidget      = nullptr;

private:

    friend class SmugWindow;
};

} // namespace DigikamGenericSmugPlugin
