var panel = new Panel
var panelScreen = panel.screen
var freeEdges = {"bottom": true, "top": true, "left": true, "right": true}

for (i = 0; i < panelIds.length; ++i) {
    var tmpPanel = panelById(panelIds[i])
    if (tmpPanel.screen == panelScreen) {
        // Ignore the new panel
        if (tmpPanel.id != panel.id) {
            freeEdges[tmpPanel.location] = false;
        }
    }
}

if (freeEdges["bottom"] == true) {
    panel.location = "bottom";
} else if (freeEdges["top"] == true) {
    panel.location = "top";
} else if (freeEdges["left"] == true) {
    panel.location = "left";
} else if (freeEdges["right"] == true) {
    panel.location = "right";
} else {
    // There is no free edge, so leave the default value
    panel.location = "top";
}

panel.height = screenGeometry(panel.screen).height > 1024 ? 35 : 27
panel.addWidget("launcher")
panel.addWidget("org.kde.showActivityManager")
pager = panel.addWidget("pager")
pager.writeConfig("hideWhenSingleDesktop", "true")
tasks = panel.addWidget("tasks")
panel.addWidget("systemtray")
panel.addWidget("digital-clock")

tasks.currentConfigGroup = new Array("Launchers")
tasks.writeConfig("browser", "preferred://browser, , , ")
tasks.writeConfig("filemanager", "preferred://filemanager, , , ")
