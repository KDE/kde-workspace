
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: digitalclock

    property int hours
    property int minutes
    property int seconds
    property string timeString
    property int year
    property int month
    property int day
    property string dateString

    property double timeScaleF: 0.7
    property double dateScaleF: 1 - timeScaleF

    property int minimumWidth
    property int minimumHeight

    function formatTime() {
        timeString = (hours + ':' + minutes + ':' + seconds)
        dateString = "Thursday 30 August 2012"
    }

    function calcFontSize(currentSize, length, paintedLength) {
        var pointS = (currentSize * (length / paintedLength));
        return Math.max(pointS, theme.smallestFont.pointSize);
    }

    function calcMinHeight() {
        var text = Qt.createQmlObject('import QtQuick 1.1; Text {}',
                                                    digitalclock, "calcSize");
        var bS, bT, lT;
        (timeScaleF < dateScaleF) ? (bS = dateScaleF, lT = timeLabel.text, bT = dateLabel.text) :
                                    (bS = timeScaleF, lT = dateLabel.text, bT = timeLabel.text);
        text.font.pointSize = theme.smallestFont.pointSize;
        text.text = lT;
        minimumHeight = text.paintedHeight;
        text.font.pointSize = theme.smallestFont.pointSize * (bS / (1 - bS))
        text.text = bT;
        minimumHeight += text.paintedHeight;
        text.destroy();
    }

    Component.onCompleted: {
        plasmoid.addEventListener("dataUpdated", dataUpdated);
        // doesn't work
//         plasmoid.addEventListener("sizeChanged", sizeChanged);
//         plasmoid.addEventListener("ConfigChanged", configChanged);
        dataEngine("time").connectSource("Local", digitalclock, 1000);
        calcMinHeight();
    }

    function dataUpdated(source, data) {
        var date = new Date("January 1, 1971 "+data.Time);
        hours = date.getHours();
        minutes = date.getMinutes();
        seconds = date.getSeconds();
        year = date.getFullYear();
        month = date.getMonth();
        day = date.getDay();
        formatTime();
        timezoneText.text = data.Timezone;
    }

    function sizeChanged() {
        timeLabel.calcSize();
        dateLabel.calcSize();
        if (plasmoid.formFactor == 2) { // Horizontal
            if (digitalclock.state == "SideBySide")
                digitalclock.width = (timeLabel.paintedWidth + dateLabel.paintedWidth);
            else
                digitalclock.width = Math.max(timeLabel.paintedWidth, dateLabel.paintedWidth);

            plasmoid.setMinimumSize(digitalclock.width, 0);
        } else if (plasmoid.formFactor == 3) { // Vertical
            digitalclock.height = timeLabel.paintedHeight + dateLabel.paintedHeight;
            plasmoid.setMinimumSize(0, digitalclock.height);
        }
    }

    PlasmaComponents.Label {
        id: timeLabel
        text: timeString

        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

        function calcSize() {
            if (plasmoid.formFactor != 3) {
                font.pointSize = calcFontSize(font.pointSize, height, paintedHeight);
                if (digitalclock.height <= minimumHeight - 15) { //squeeze it!
                    digitalclock.state = 'SideBySide';
                    width = paintedWidth;
                    height = digitalclock.height * 1;
                } else if (digitalclock.height >= minimumHeight - 15) {
                    digitalclock.state = 'Horizontal';
                    height = digitalclock.height * timeScaleF;
                }
            } else {
                font.pointSize = calcFontSize(font.pointSize, width, paintedWidth);
                height = paintedHeight;
            }
        }

        anchors {
            top: digitalclock.top
            left: digitalclock.left
            right: digitalclock.right
        }

        Rectangle {
            color: "red"
            opacity: 0.5

            anchors.fill: parent
        }
    }

//     PlasmaComponents.Label {
    Text {
        id: dateLabel
        text: dateString

        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

        function calcSize() {
            var pointS;
            font.pointSize = theme.smallestFont.pointSize;
            if (plasmoid.formFactor != 3) {
                if (digitalclock.state != 'SideBySide') {
                    pointS = calcFontSize(font.pointSize, height, paintedHeight);
                    anchors.leftMargin = 0;
                }
                else {
                    pointS = theme.smallestFont.pointSize;
                    anchors.leftMargin = 5;
                }
            } else if (plasmoid.formFactor == 3) {
                if (dateLabel.lineCount > 1)
                    return;

                pointS = calcFontSize(font.pointSize, width, paintedWidth);
            }
            font.pointSize = pointS;
        }
        anchors {
            top: timeLabel.bottom
            left: digitalclock.left
            bottom: digitalclock.bottom
            right: digitalclock.right
        }
        Rectangle {
            color: "blue"
            opacity: 0.5

            anchors.fill: parent
        }
    }

    onWidthChanged: {
        sizeChanged();
    }
    onHeightChanged: {
        sizeChanged();
    }

    states: [
        State {
            name: "Vertical"
            when: (plasmoid.formFactor == 3)
            PropertyChanges {
                target: dateLabel
                maximumLineCount: 4
                wrapMode: Text.Wrap
                anchors.bottomMargin: 0
            }
        },
        State {
            name: "SideBySide"
            PropertyChanges {
                target: dateLabel
                maximumLineCount: 1
                wrapMode: Text.NoWrap
                anchors.bottomMargin: 0
            }
            AnchorChanges {
                target: timeLabel
                anchors {
                    right: undefined
                }
            }
            AnchorChanges {
                target: dateLabel
                anchors {
                    top: digitalclock.top
                    left: timeLabel.right
                    bottom: digitalclock.bottom
                    right: digitalclock.right
                }
            }
        },
        State {
            name: "Horizontal"
            PropertyChanges {
                target: dateLabel
                maximumLineCount: 1
                wrapMode: Text.NoWrap
                anchors.bottomMargin: 5
            }
        }
    ]
}