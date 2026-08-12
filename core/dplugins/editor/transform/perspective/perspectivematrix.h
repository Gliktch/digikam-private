/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a matrix implementation for image
 *               perspective adjustment.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

namespace DigikamEditorPerspectiveToolPlugin
{

class PerspectiveMatrix
{
public:

    /**
     * PerspectiveMatrix:
     *
     * Initializes matrix to the identity matrix.
     */
    PerspectiveMatrix();

    /**
     * translate:
     * @param x Translation in X direction.
     * @param y Translation in Y direction.
     *
     * Translates the matrix by x and y.
     */
    void translate(double x, double y);

    /**
     * scale:
     * @param x X scale factor.
     * @param y Y scale factor.
     *
     * Scales the matrix by x and y
     */
    void scale(double x, double y);

    /**
     * invert:
     *
     * Inverts this matrix.
     */
    void invert();

    /**
     * multiply:
     * @param matrix1 The other input matrix.
     *
     * Multiplies this matrix with another matrix
     */
    void multiply(const PerspectiveMatrix& matrix1);

    /**
     * transformPoint:
     * @param x The source X coordinate.
     * @param y The source Y coordinate.
     * @param newx The transformed X coordinate.
     * @param newy The transformed Y coordinate.
     *
     * Transforms a point in 2D as specified by the transformation matrix.
     */
    void transformPoint(double x, double y, double* newx, double* newy) const;

    /**
     * determinant:
     *
     * Calculates the determinant of this matrix.
     *
     * Returns: The determinant.
     */
    double determinant() const;

public:

    /**
     * coeff:
     *
     * The 3x3 matrix data
     */
    double coeff[3][3] = { { 0.0 } };
};

} // namespace DigikamEditorPerspectiveToolPlugin
