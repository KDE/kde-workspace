
var panel = new Panel
panel.screen = 0
panel.location = 'bottom'
panel.addWidget("org.kde.kickoff")
panel.addWidget("org.kde.pager")
panel.addWidget("org.kde.plasma.taskmanager")
panel.addWidget("org.kde.systemtray")
panel.addWidget("org.kde.digitalclock")

for (var i = 0; i < screenCount; ++i) {
    var desktop = new Activity
    desktop.name = i18n("Desktop")
    desktop.screen = i
    desktop.wallpaperPlugin = 'org.kde.image'

    //var clock = desktop.addWidget("org.kde.analogclock");
}
