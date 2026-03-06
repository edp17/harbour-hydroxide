import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.hydroxide 1.0
import "pages"
import "cover" as Covers

ApplicationWindow {
    initialPage: Component { MainPage {} }
    cover: Component { Covers.CoverPage { } }
}
