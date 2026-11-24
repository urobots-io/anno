"""Generate the application icons of anno from Fluent UI System Icons.

Icons: https://github.com/microsoft/fluentui-system-icons (MIT license,
see Resources/LICENSE-fluentui-system-icons.txt).

Every icon is rendered from the SVG drawn for the closest size (Fluent icons
are hand-tuned for 16, 20, 24, 32 and 48 px) and written as a multi-size .ico
file into ../Resources, replacing the file with the same name.

Requirements: Python 3.9+, PySide6 (Qt SVG renderer) and Pillow.
    pip install PySide6-Essentials Pillow
    python generate_icons.py
"""

import io
import os
import sys
import urllib.request
import urllib.error

from PIL import Image
from PySide6.QtCore import QByteArray, QPointF, Qt
from PySide6.QtGui import QColor, QGuiApplication, QImage, QPainter, QPolygonF
from PySide6.QtSvg import QSvgRenderer

FLUENT_VERSION = "1.1.347"
FLUENT_URL = "https://cdn.jsdelivr.net/npm/@fluentui/svg-icons@{version}/icons/{name}_{size}_regular.svg"

# sizes stored in every .ico file (100% to 300% display scaling)
ICO_SIZES = [16, 20, 24, 32, 40, 48]
# sizes in which Fluent icons are drawn
FLUENT_SIZES = [16, 20, 24, 28, 32, 48]

GRAY = "#424242"   # default icon color
BLUE = "#0F6CBD"   # save, undo / redo
GREEN = "#107C10"  # add, evaluate
RED = "#C42B1C"    # delete, remove
AMBER = "#C98500"  # folders, new / open project
LIGHT = "#F2F2F2"        # icons on dark backgrounds (Toolbox header)
LIGHT_GREEN = "#6CCB5F"  # add on dark backgrounds

# file name in Resources: (Fluent icon name, color)
ICONS = {
    "new.ico": ("document_add", AMBER),
    "open.ico": ("folder_open", AMBER),
    "folder_network.ico": ("database", GRAY),
    "save.ico": ("save", BLUE),
    "Undo.ico": ("arrow_undo", BLUE),
    "Redo.ico": ("arrow_redo", BLUE),
    "search-inside.ico": ("scan_object", GREEN),
    "3d.ico": ("cube", GRAY),
    "configuration.ico": ("settings", GRAY),
    "fit_to_size.ico": ("full_screen_maximize", GRAY),
    "nav_back.ico": ("arrow_left", GRAY),
    "nav_forward.ico": ("arrow_right", GRAY),
    "add.ico": ("add", GREEN),
    "delete.ico": ("delete", RED),
    "clean.ico": ("eraser", RED),
    "copy.ico": ("copy", GRAY),
    "paste.ico": ("clipboard_paste", GRAY),
    "rename.ico": ("rename", GRAY),
    "refresh.ico": ("arrow_clockwise", GRAY),
    "restart.ico": ("arrow_reset", GRAY),
    "tree_view.ico": ("text_bullet_list_tree", GRAY),
    "select.ico": ("cursor", GRAY),
    "folder.ico": ("folder", AMBER),
    # image files in the file tree: without and with markers
    "file.ico": ("document", GRAY),
    "file_labeled.ico": ("document", GRAY, "markers"),
    # Toolbox header buttons, drawn on a dark background
    "copy_light.ico": ("copy", LIGHT),
    "paste_light.ico": ("clipboard_paste", LIGHT),
    "configuration_light.ico": ("settings", LIGHT),
    "add_light.ico": ("add", LIGHT_GREEN),
    "tree_view_light.ico": ("text_bullet_list_tree", LIGHT),
}


def download(name, size):
    url = FLUENT_URL.format(version=FLUENT_VERSION, name=name, size=size)
    request = urllib.request.Request(url, headers={"User-Agent": "anno-icon-generator"})
    try:
        with urllib.request.urlopen(request, timeout=30) as response:
            return response.read()
    except urllib.error.HTTPError as e:
        if e.code == 404:
            return None
        raise


def render(svg, size, color):
    # Fluent SVGs have no fill color, set it on the root element
    svg = svg.replace(b"<svg ", ('<svg fill="%s" ' % color).encode(), 1)
    renderer = QSvgRenderer(QByteArray(svg))
    if not renderer.isValid():
        raise RuntimeError("invalid SVG")
    image = QImage(size, size, QImage.Format_ARGB32_Premultiplied)
    image.fill(Qt.transparent)
    painter = QPainter(image)
    painter.setRenderHint(QPainter.Antialiasing)
    renderer.render(painter)
    painter.end()
    return image


def qimage_to_pil(image):
    size = image.width()
    image = image.convertToFormat(QImage.Format_RGBA8888)
    return Image.frombuffer("RGBA", (size, size), bytes(image.constBits()), "raw", "RGBA", 0, 1).copy()


# "markers" overlay: a red circle over a blue triangle (like markers on an image),
# drawn into the free lower part of the Fluent "document" page.
# Area (left, top, right, bottom) in SVG units for every drawn size of "document".
MARKERS_AREA = {
    16: (4.5, 6.5, 11.5, 13.5),
    20: (5.5, 7.5, 14.5, 16.5),
    24: (6.0, 10.5, 18.0, 20.0),
    32: (8.0, 14.0, 24.0, 27.0),
    48: (11.5, 21.0, 36.5, 40.5),
}


def draw_markers(image, svg_size):
    left, top, right, bottom = MARKERS_AREA[svg_size]
    scale = image.width() / svg_size
    left, top, right, bottom = left * scale, top * scale, right * scale, bottom * scale
    w, h = right - left, bottom - top

    painter = QPainter(image)
    painter.setRenderHint(QPainter.Antialiasing)
    painter.setPen(Qt.NoPen)
    painter.setBrush(QColor(BLUE))
    painter.drawPolygon(QPolygonF([
        QPointF(left + 0.62 * w, top),
        QPointF(right, bottom),
        QPointF(left + 0.22 * w, bottom)]))
    painter.setBrush(QColor(RED))
    radius = 0.3 * min(w, h)
    painter.drawEllipse(QPointF(left + radius, bottom - radius), radius, radius)
    painter.end()


def source_size(target, available):
    if target in available:
        return target
    larger = [s for s in available if s > target]
    return min(larger) if larger else max(available)


def main():
    app = QGuiApplication(sys.argv[:1] + ["-platform", "offscreen"])
    resources = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Resources")

    cache = {}
    for file_name, (icon_name, color, *overlay) in ICONS.items():
        if icon_name not in cache:
            cache[icon_name] = {s: download(icon_name, s) for s in FLUENT_SIZES}
            cache[icon_name] = {s: v for s, v in cache[icon_name].items() if v}
            if not cache[icon_name]:
                sys.exit("Fluent icon not found: " + icon_name)
        svgs = cache[icon_name]
        if overlay:
            # use only the drawn sizes with a known overlay area
            svgs = {s: v for s, v in svgs.items() if s in MARKERS_AREA}

        images = []
        for size in ICO_SIZES:
            svg_size = source_size(size, svgs.keys())
            image = render(svgs[svg_size], size, color)
            if overlay:
                draw_markers(image, svg_size)
            images.append(qimage_to_pil(image))
        largest = images[-1]
        largest.save(
            os.path.join(resources, file_name),
            format="ICO",
            sizes=[(s, s) for s in ICO_SIZES],
            append_images=images[:-1])
        print("%-20s %-24s %s  drawn sizes %s" % (file_name, icon_name, color, sorted(svgs)))

    del app


if __name__ == "__main__":
    main()
