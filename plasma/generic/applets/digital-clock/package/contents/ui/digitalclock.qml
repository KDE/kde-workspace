
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.locale 0.1 as KLocale

Item {
    id: digitalclock

    property variant dateTime
    property variant dateStyle
    property bool showSeconds

    property double timeScaleF: 0.7
    property double dateScaleF: 1 - timeScaleF

    property int minimumWidth
    property int minimumHeight

    function twoDigitString(number)
    {
        return number < 10 ? "0"+number : number
    }

    function calcFontSize(currentSize, length, paintedLength) {
        var pointS = (currentSize * (length / paintedLength));
        return Math.max(pointS, theme.smallestFont.pointSize);
    }

    function sizeChanged() {
        if (plasmoid.formFactor == Horizontal) {
            if (digitalclock.height < minimumHeight) {
                digitalclock.state = "SideBySide";
                digitalclock.width = (timeLabel.paintedWidth + dateLabel.paintedWidth);
            } else {
                digitalclock.state = "Horizontal";
                digitalclock.width = Math.max(timeLabel.paintedWidth, dateLabel.paintedWidth);
            }
            plasmoid.setMinimumSize(digitalclock.width, 0);
        } else if (plasmoid.formFactor == Vertical) {
            digitalclock.height = timeLabel.paintedHeight + dateLabel.paintedHeight;
            plasmoid.setMinimumSize(0, digitalclock.height);
            (dateLabel.lineCount == 1) ? dateLabel.font.pixelSize = calcFontSize(dateLabel.font.pixelSize, dateLabel.width - 10, dateLabel.paintedWidth) :
                                         dateLabel.font.pointSize = 10;
        } else { // Planar
            var maxWidth = Math.max(timeLabel.paintedWidth, dateLabel.paintedWidth);
            //whats a reasonable smallest pixelSize?
            if (Math.min(timeLabel.font.pixelSize, dateLabel.font.pixelSize) <= 10) {
                minimumWidth = maxWidth;
                minimumHeight = timeLabel.height + dateLabel.height;
            } else { //FIXME not very smooth
                if (maxWidth > digitalclock.width) {
                    minimumWidth = digitalclock.width + 1;
                } else {
                    minimumWidth = maxWidth - 1;
                    minimumHeight = 0;
                }
            }
        }
    }

    function configChanged() {
        showSeconds = plasmoid.readConfig("showSeconds");
        switch(Number(plasmoid.readConfig("dateStyle"))) {
            case 0:
                dateStyle = KLocale.Locale.ShortDate;
                break;
            case 1:
                dateStyle = KLocale.Locale.ShortDate;
                break;
            case 2:
                dateStyle = KLocale.Locale.ShortDate;
                break;
            case 3:
                dateStyle = KLocale.Locale.LongDate;
                break;
            case 4:
                dateStyle = KLocale.Locale.IsoDate;
                break;
            default:
                dateStyle = KLocale.Locale.ShortDate;
        }
        print("configChanged " + showSeconds + ' ' + dateStyle);
    }

    PlasmaCore.DataSource {
        id: clockSource
        engine: "time"
        interval: 1000
        connectedSources: ["Local"]
        onDataChanged: dateTime = new Date(data["Local"]["DateTime"]);
    }

    Component.onCompleted: {
        plasmoid.addEventListener("ConfigChanged", configChanged);
        plasmoid.aspectRatioMode = IgnoreAspectRatio;
    }

    KLocale.Locale {
        id: locale
    }

    PlasmaComponents.Label {
        id: timeLabel
        text: twoDigitString(dateTime.getHours()) + ":" + twoDigitString(dateTime.getMinutes()) +
            (showSeconds ? ":" + twoDigitString(dateTime.getSeconds()) : "")

        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

        height: digitalclock.height * timeScaleF
        font.pixelSize: height

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

    PlasmaComponents.Label {
        id: dateLabel
        text: locale.formatDate(dateTime, dateStyle)

        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

        font.pixelSize: height

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
            when: (plasmoid.formFactor == Vertical)
            PropertyChanges {
                target: dateLabel
                maximumLineCount: 4
                wrapMode: Text.Wrap
                font.pixelSize: undefined
            }
            PropertyChanges {
                target: timeLabel
                // FIXME binding loop...
                font.pixelSize: calcFontSize(font.pixelSize, width, paintedWidth)
                height: paintedHeight
            }
        },
        State {
            name: "SideBySide"
            PropertyChanges {
                target: dateLabel
                maximumLineCount: 1
                wrapMode: Text.NoWrap
            }
            PropertyChanges {
                target: timeLabel
                height: digitalclock.height
                width: paintedWidth
            }
            AnchorChanges {
                target: timeLabel
                anchors {
                    right: undefined
                    bottom: digitalclock.bottom
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
                // fixes binding loop
                height: digitalclock.height - timeLabel.height
            }
        }
    ]
}