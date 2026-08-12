/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View - Private containers and classes.
 *
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "labelstreeview_p.h"

namespace Digikam
{

ColorLabelValidator::ColorLabelValidator(int maxLength, QObject* const parent)
    : QValidator (parent),
      m_maxLength(maxLength)
{
}

QValidator::State ColorLabelValidator::validate(QString& input, int& /*pos*/) const
{
    if (input.length() <= m_maxLength)
    {
        return QValidator::Acceptable;
    }

    return QValidator::Invalid;
}

// ---

ColorLabelDelegate::ColorLabelDelegate(int maxLength, QObject* const parent)
    : QStyledItemDelegate(parent),
      m_maxLength        (maxLength)
{
}

QWidget* ColorLabelDelegate::createEditor(QWidget* parent,
                                          const QStyleOptionViewItem& /*option*/,
                                          const QModelIndex& /*index*/) const
{
    QLineEdit* const editor = new QLineEdit(parent);
    editor->setValidator(new ColorLabelValidator(m_maxLength, editor));
    editor->setClearButtonEnabled(true);
    editor->setPlaceholderText(i18n("Customize label name here..."));

    return editor;
}

void ColorLabelDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QString text              = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit* const lineEdit = qobject_cast<QLineEdit*>(editor);
    lineEdit->setText(text);
}

void ColorLabelDelegate::setModelData(QWidget* editor,
                                      QAbstractItemModel* model,
                                      const QModelIndex& index) const
{
    QLineEdit* const lineEdit = qobject_cast<QLineEdit*>(editor);
    QString text              = lineEdit->text();

    if (text.isEmpty())
    {
        // If empty, reset to default name.

        int label = index.model()->data(index, Qt::UserRole).toInt();
        text      = ColorLabelWidget::labelColorName((ColorLabel)label);
    }

    model->setData(index, text, Qt::EditRole);
}

// ---

LabelsTreeView::Private::Private(QTreeWidget* const w)
    : itemIterator(w)
{
}

} // namespace Digikam
