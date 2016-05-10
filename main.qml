import QtQuick 2.0

Item {
     Rectangle {
         anchors.fill: parent
         color: "red"
         NumberAnimation on opacity {
             from: 1.0
             to: 0.0
             duration: 5000
         }
     }
}
