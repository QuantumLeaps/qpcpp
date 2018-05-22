import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Window {
    visible: true
    width: 100
    height: 480
    title: qsTr("DPP")

    ColumnLayout {

        Repeater {
            model:5
            Image {
                source: encodeURIComponent(gui.state[index])
            }
        }

        Button {
            text: gui.button
            onPressed: {
                gui.onPausePressed()
            }
            onReleased: {
                gui.onPauseReleased()
            }
        }

        Button {
            text: "QUIT"
            onClicked: {
                gui.onQuit()
            }
        }
    }
}
