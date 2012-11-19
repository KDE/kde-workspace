/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 *
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
 * License: LGPLv2+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/

import QtQuick 1.1

Item {
    id: root_item

    property int icons_size:     24  ///< Size of icons, icons are square i.e. width == height
    property int icons_margins:  2  ///< Margins for icons
    property alias icons_number: grid.count  ///< [readonly] Number of icons
    property alias model:    grid.model; ///< Model for grid
    property int cols_num: 0 ///< [readonly] Number of columns
    property int rows_num: 0 ///< [readonly] Number of rows
    property int min_width:   cols_num*cell_size  ///< [readonly] minimum width of component required to show whole grid
    property int min_height:  rows_num*cell_size  ///< [readonly] minimum height of compontn required to show whole grid
    property int cell_size: icons_size + 2*icons_margins ///< [readonly] size of grid cell

    property int _sqrt: 0
    property int _sqrt2: _sqrt*_sqrt

    GridView {
        id: grid
        anchors.centerIn: parent
        width:  min_width
        height: min_height

        cellWidth: cell_size
        cellHeight: cellWidth
        interactive: false
        delegate: TrayIcon { width: grid.cellWidth; height: grid.cellHeight }
    }

    states: [
        /// Base state for square states (for internal use only)
        State {
            name: "__SQR"

            PropertyChanges {
                target: root_item
                _sqrt: Math.floor(Math.sqrt(grid.count))
            }
        },

        /// Vertical layout of grid
        State {
            name: "VERT"

            PropertyChanges {
                target: root_item
                _sqrt: 0
                cols_num: grid.count*cell_size <= width ? grid.count : Math.max(1, Math.floor(width / cell_size))
                rows_num: cols_num > 0 ? (Math.max(1, Math.floor(grid.count / cols_num)) + (grid.count % cols_num ? 1 : 0)) : 0
            }
            PropertyChanges {
                target: grid
                flow:   GridView.LeftToRight
            }
        },

        /// Horizontal layout of grid
        State {
            name: "HORZ"

            PropertyChanges {
                target: root_item
                _sqrt: 0
                rows_num: grid.count*cell_size <= height ? grid.count : Math.max(1, Math.floor(height / cell_size))
                cols_num: rows_num > 0 ? (Math.max(1, Math.floor(grid.count / rows_num)) + (grid.count % rows_num ? 1 : 0)) : 0
            }
            PropertyChanges {
                target: grid
                flow:   GridView.TopToBottom
            }
        },

        /// Square layout of grid with horizontal flow
        State {
            name: "SQR_H"
            extend: "__SQR"

            PropertyChanges {
                target: root_item
                cols_num: ( grid.count > _sqrt2   ? _sqrt + 1 : _sqrt )
                rows_num: ( grid.count > _sqrt2 + _sqrt ? _sqrt + 1 : _sqrt )
            }
            PropertyChanges {
                target: grid
                flow:   GridView.LeftToRight
            }
        },

        /// Square layout of grid with vertical flow
        State {
            name: "SQR_V"
            extend: "__SQR"
            PropertyChanges {
                target: root_item
                rows_num: ( grid.count > _sqrt2   ? _sqrt + 1 : _sqrt )
                cols_num: ( grid.count > _sqrt2 + _sqrt ? _sqrt + 1 : _sqrt )
            }
            PropertyChanges {
                target: grid
                flow:   GridView.TopToBottom
            }
        }
    ]
}
