var panel = new Panel
var panelScreen = panel.screen
var numberOfPanelsOnScreen = 0

for (i = 0; i < panelIds.length; ++i) {
    if (panelById(panelIds[i]).screen == panelScreen) {
        numberOfPanelsOnScreen += 1
    }
}

if (numberOfPanelsOnScreen == 1) {
    // we are the only panel, so set the location for the user
    panel.location = 'bottom'
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
