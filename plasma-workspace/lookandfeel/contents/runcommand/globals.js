/*
 * This file is part of the KDE Milou Project
 * Copyright (C) 2014 Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

var isHighDPI = theme.defaultFont.pixelSize >= 20

// Normal
var CategoryWidthNormal = 120
var PlasmoidWidthNormal = 450
var IconSizeNormal = 16
var TitleSizeNormal = 32

// High DPI
var CategoryWidthHighDPI = 220
var PlasmoidWidthHighDPI = 650
var IconSizeHighDPI = 32
var TitleSizeHighDPI = 64

// This is the width of the side component which displays the categories
// of the results, such as "Applications", "Audio", "Video", etc
var CategoryWidth = isHighDPI ? CategoryWidthHighDPI : CategoryWidthNormal
var CategoryRightMargin = 10

// The Maximum and Minimum width of the Plasmoid
var PlasmoidWidth = isHighDPI ? PlasmoidWidthHighDPI : PlasmoidWidthNormal

var IconSize = isHighDPI ? IconSizeHighDPI : IconSizeNormal
var TitleSize = isHighDPI ? TitleSizeHighDPI : TitleSizeNormal
