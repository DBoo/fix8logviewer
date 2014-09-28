import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.0
import QtQuick.Controls.Styles 1.2
TabView {
    id:tv
    property color bgColor:"green"
    property string version:"1.2"
    function setVersion(v)
    {
        version = v;
    }
    style: TabViewStyle {

        frameOverlap: 1

        tabBar: Rectangle { color: tv.bgColor; anchors.fill: parent }
    }
    anchors.fill: parent
    Component.onCompleted: {
        addTab("Version",versionTab)
        addTab("Description", descriptionTab)
        addTab("Author", authorTab)
        addTab("Community",communityTab)
        addTab("License", licenseTab)
    }
    Component {
        id: versionTab
        Rectangle {
            id:versionRec
            anchors.fill: parent
            anchors.margins: 24
            radius: 8
            Text  {
                id:versionTitle
                text: "Fix8LogViewer Version"
                color:"black"
                font.bold: true;
                font.pointSize: 18
                anchors.top: parent.top
                anchors.topMargin: 100
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id:versionText
                text:tv.version
                color:"black"
                font.pointSize: 18
                font.bold: true
                anchors.top:versionTitle.bottom
                anchors.topMargin: 18
                anchors.horizontalCenter: parent.horizontalCenter



            }
        }
    }
    Component {
        id: descriptionTab
        Rectangle {
            id:descriptionRec
            anchors.fill: parent
            anchors.margins: 24
            radius: 8

            TextArea {
                frameVisible: false
                id:textArea
                anchors.fill: parent
                anchors.topMargin: 22
                anchors.rightMargin: 12
                anchors.leftMargin: 12
                font.bold: true
                font.pointSize: 15
                textFormat:Text.RichText
                text: '<html> <body> <i>Fix8logviewer</i> is an open source FIX log file viewer. These log files need to be in xml format. <i>Fix8logviewer</I> ships with several FIX schemas but also allows the user to add their own custome FIX schemas. Please refer to the FIX8 community website to find out how to support custome schemas.</body></html> '
                readOnly: true
            }
        }
    }
    Component {
        id: authorTab
        Rectangle  {
            anchors.margins: 24
            anchors.fill: parent
            radius:8
            TextArea {
                id: name
                frameVisible: false
                font.bold: true
                font.pointSize: 15
                readOnly: true
                textFormat:Text.RichText
                text: '<html> <body> <i>Fix8logviewer</i> was developed and maintained by <b>David Boosalis</b>.  Please feel free to contact David if you have any questions or suggestions.  David is also available for consulting work.<p> <a href="mailto://david.boosalis@gmail.com">david.boosalis@gmail.com</a>'
                onLinkActivated:  Qt.openUrlExternally("mailTo:david.booslis@gmail.com")
                anchors.fill: parent
                anchors.topMargin: 22
                anchors.rightMargin: 12
                anchors.leftMargin: 12
            }
        }
    }
    Component {
        id: communityTab
        Rectangle {
            id:jiraRect
            radius:8
            anchors.fill: parent
            anchors.margins: 24
            TextArea {
                id:jiraText
                frameVisible: false
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.left: parent.left
                height: 100
                anchors.margins: 12
                font.bold: true
                font.pointSize: 14
                readOnly: true
                textFormat: TextEdit.RichText
                text:  '<html> <body> For suggestions, assistance, or to report a bug please visit <a href="www.fix8.org/support.html">Fix8 Community Forum/Tools</a></body></html>'
                onLinkActivated:  Qt.openUrlExternally("http://www.fix8.org/support")
            }
        }
    }
    Component {
        id: licenseTab
        Rectangle {
            id:licRect
            radius:8
            anchors.fill: parent
            anchors.margins: 24
            TextArea {
                id:licText
                anchors.fill: parent
                anchors.margins: 24
                font.bold: true
                font.pointSize: 15
                readOnly: true
                textFormat: TextEdit.RichText
                text:  '<html> <body> This product is released under the <a href="https://www.gnu.org/licenses/quick-guide-gplv3.html">GNU LESSER GENERAL PUBLIC LICENSE Version 3 </a></body></html>'
                onLinkActivated:  Qt.openUrlExternally("https://www.gnu.org/licenses/quick-guide-gplv3.html")
            }
        }
    }
}

