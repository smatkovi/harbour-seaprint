import QtQuick 1.1
Item {
    property variant currentPage
    property int depth: 0
    function push(page, properties, immediate) { return page }
    function pop(page, immediate) {}
    function replace(page, properties, immediate) { return page }
    function clear() {}
}
