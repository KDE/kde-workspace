for (var i in panelIds) {
    var panel = panelById(panelIds[i]);
    for (var j in panel.widgetIds) {
        var widget = panel.widgetById(panel.widgetIds[j]);
        if (widget.type == "pager") {
            widget.writeConfig('hideWhenSingleDesktop', 'true');
            widget.reloadConfig();
        }
    }
}
