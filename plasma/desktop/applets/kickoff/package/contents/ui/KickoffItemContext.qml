
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.kickoff 0.1 as Kickoff

Item {
    property alias menu: menu
    property alias addToFavorites: addToFavorites
    property alias removeFromFavorites: removeFromFavorites
    property alias separator: separator
    property alias sortFavoritesAscending: sortFavoritesAscending
    property alias sortFavoritesDescending: sortFavoritesDescending

    PlasmaComponents.ContextMenu {
        id: menu
    }
    /*
     * context menu items
     */
    PlasmaComponents.MenuItem {
        id: addToFavorites
        text: "Add to favorites"
        icon: QIcon("list-add")
        onClicked: {
            listItem.ListView.view.parent.favoritesModel.add(model["url"]);
        }
    }
    PlasmaComponents.MenuItem {
        id: removeFromFavorites
        text: "Remove from favorites"
        icon: QIcon("list-remove")
        onClicked: {
            listItem.ListView.view.model.remove(model["url"]);
        }
    }
    PlasmaComponents.MenuItem {
        id: separator
        separator: true
    }
    PlasmaComponents.MenuItem {
        id: sortFavoritesAscending
        text: "Sort Alphabetically (A to Z)"
        icon: QIcon("view-sort-ascending")
        onClicked: {
            listItem.ListView.view.model.sortFavoritesAscending();
        }
    }
    PlasmaComponents.MenuItem {
        id: sortFavoritesDescending
        text: "Sort Alphabetically (Z to A)"
        icon: QIcon("view-sort-descending")
        onClicked: {
            listItem.ListView.view.model.sortFavoritesDescending();
        }
    }
}