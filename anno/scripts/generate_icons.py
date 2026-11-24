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

import os
import sys
import urllib.request
import urllib.error

from PIL import Image
from PySide6.QtCore import QByteArray, Qt
from PySide6.QtGui import QGuiApplication, QImage, QPainter
from PySide6.QtSvg import QSvgRenderer

FLUENT_VERSION = "1.1.347"
FLUENT_URL = "https://cdn.jsdelivr.net/npm/@fluentui/svg-icons@{version}/icons/{name}_{size}_{style}.svg"

# sizes stored in every .ico file (100% to 300% display scaling)
ICO_SIZES = [16, 20, 24, 32, 40, 48]
# sizes in which Fluent icons are drawn
FLUENT_SIZES = [16, 20, 24, 28, 32, 48]

GRAY = "#424242"   # default icon color
BLUE = "#0F6CBD"   # save, undo / redo, image files with labels
GREEN = "#107C10"  # add, evaluate
RED = "#C42B1C"    # delete, remove
AMBER = "#C98500"  # folders, new / open project
LIGHT = "#F2F2F2"        # icons on dark backgrounds (Toolbox header)
LIGHT_GREEN = "#6CCB5F"  # add on dark backgrounds

# file name in Resources: (Fluent icon name, color[, style]), style "regular" (default) or "filled"
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
    # image files in the file tree: outlined page without labels, filled page with labels
    "file.ico": ("document", GRAY),
    "file_labeled.ico": ("document", BLUE, "filled"),
    # image toolbar: image properties overlay, display settings
    "image_info.ico": ("info", GRAY),
    "image_display.ico": ("brightness_high", GRAY),
    # Toolbox header buttons, drawn on a dark background
    "copy_light.ico": ("copy", LIGHT),
    "paste_light.ico": ("clipboard_paste", LIGHT),
    "configuration_light.ico": ("settings", LIGHT),
    "add_light.ico": ("add", LIGHT_GREEN),
    "tree_view_light.ico": ("text_bullet_list_tree", LIGHT),
}


def download(name, size, style):
    url = FLUENT_URL.format(version=FLUENT_VERSION, name=name, size=size, style=style)
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


def source_size(target, available):
    if target in available:
        return target
    larger = [s for s in available if s > target]
    return min(larger) if larger else max(available)


def main():
    app = QGuiApplication(sys.argv[:1] + ["-platform", "offscreen"])
    resources = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Resources")

    cache = {}
    for file_name, (icon_name, color, *options) in ICONS.items():
        style = options[0] if options else "regular"
        key = (icon_name, style)
        if key not in cache:
            svgs = {s: download(icon_name, s, style) for s in FLUENT_SIZES}
            cache[key] = {s: v for s, v in svgs.items() if v}
            if not cache[key]:
                sys.exit("Fluent icon not found: %s (%s)" % (icon_name, style))
        svgs = cache[key]

        images = [qimage_to_pil(render(svgs[source_size(size, svgs.keys())], size, color)) for size in ICO_SIZES]
        largest = images[-1]
        largest.save(
            os.path.join(resources, file_name),
            format="ICO",
            sizes=[(s, s) for s in ICO_SIZES],
            append_images=images[:-1])
        print("%-24s %-24s %-8s %s  drawn sizes %s" % (file_name, icon_name, style, color, sorted(svgs)))

    del app


if __name__ == "__main__":
    main()
