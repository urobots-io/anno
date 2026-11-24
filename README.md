# anno
Cross-platform tool for labeling image data.
[Video introduction to anno](https://youtu.be/RrjGU2vSIOM).

<img src="https://user-images.githubusercontent.com/9085494/89163912-a8055380-d576-11ea-8f8d-da664c6f4687.png"/>

## Features
- Supports a wide range of marker types:
  - Point
  - Oriented point
  - Circle
  - Oriented circle
  - Axis-aligned rectangle
  - Rectangle with rotation
  - Polygon (with holes)
  - Polyline
  - Measuring tool (distance and angle)
- Configurable rendering of all marker types, including custom rendering scripts, see [Marker rendering](#marker-rendering).
- Configurable dependencies between properties of markers, e.g. markers can share their width, height, radius, angle or their whole geometry.
- Supports a wide variety of image formats. The pre-built Windows version supports the EXR format.
- The project file is a JSON file which contains project-specific marker definitions and labeling information.
- The user interface is optimized for fast sub-pixel accurate labeling.

<img src="https://user-images.githubusercontent.com/9085494/89164042-d5520180-d576-11ea-8a74-f4a13a6716d7.gif"/>

## Requirements
- Windows / macOS / Ubuntu

## Installation
There are two options:
- Pre-built binaries from [the release section](https://github.com/urobots-io/anno/releases).
- Cross-platform build is possible with Qt Creator from the open source [Qt setup](https://www.qt.io/).

## Specifications
Project file format:
```jsonc
{
    /** Project definitions. */
    "definitions": {
        /** Folder with images, absolute path or relative path to the project file. */
        "files_root_dir": "c:/images",
        /** A dictionary with marker type definitions. */
        "marker_types": {
            /** Key represents a unique marker type name. */
            "my_marker_type": {
                /** Optional description of the marker type. */
                "description": "use carefully",
                /** Type of the marker. The following types are supported:
                    - circle
                    - oriented_circle
                    - oriented_point
                    - oriented_rect    
                    - point
                    - polygon
                    - polyline
                    - rect
                    - tool
                */
                "value_type": "rect",
                /** List of categories */ 
                "categories": [
                    {
                        /** Numerical id of the category */
                        "id": 0,
                        /** Human-readable name of the category */
                        "name": "Ok",
                        /** Color of the category */
                        "color": "red"
                    }
                ],
                
                /** Optional width of lines used to draw the marker. 
                    A negative value defines a scale-independent width in screen pixels,
                    a positive value defines a width in image pixels. */
                "line_width": -3,
                
                /** Advanced parameters: stamp, stamp_parameters, axis_length, 
                    rendering_script, shared, shared_count, 
                    filename_filter, shared_properties, 
                    custom_properties (see the JSON Schema below)
                */
            }
        },
        /** Optional object, can be used to store user information associated with
            the project. anno stores it unchanged; only the optional fields
            "evaluate_url" and "evaluate_url_args" are used by the Evaluate function. */
        "user_data": {
        }
    },

    /** List of all labeled files with their markers. */
    "files": [
        {
            /** Path to the image, relative to the files_root_dir. */
            "name": "image.png",
            /** List of all markers present on the image. */
            "markers": [
                {
                    /** Category id of the marker. */
                    "category": 0,
                    /** Marker type name. */
                    "type": "my_marker_type",
                    /** Marker geometry, see below. */
                    "value": "642.9 580.2 936.8 674.4"

                    /** Optional fields: text, shared_index, children, custom_properties
                        (see the JSON Schema below) */
                }
            ]
        }
    ]
}
```

The format of the marker geometry depends on the `value_type` of the marker type:
* circle: ```center_x center_y radius```
* oriented_circle: ```center_x center_y angle radius```
* oriented_point: ```x y angle```
* oriented_rect: ```center_x center_y width height angle```
* point: ```x y```
* polygon: ```x0 y0 x1 y1 ... xN yN```
* polyline: ```x0 y0 x1 y1 ... xN yN```
* rect: ```x0 y0 x1 y1```
* tool: ```x0 y0 x1 y1 ... xN yN``` (2 points: distance, 3 points: angle, 4 points: angle between two lines)

All coordinates are in image pixels, angles are in radians.
Additional contours of a polygon (holes) are stored in `children`, every child has its own `value`.

### Marker rendering
Every marker type has a default rendering: anno draws the geometry of the marker (rectangle, circle, polygon, ...)
in the color of its category. The default rendering can be adjusted with the fields below,
or completely overwritten with a custom [rendering script](#rendering-script).

How markers are drawn is configured per marker type in `definitions.marker_types`:

| Field | Effect |
|---|---|
| `categories[].color` | Color of the marker. Without it, a standard color is chosen by category id. |
| `line_width` | Line width. Negative: in screen pixels, independent of the zoom (default `-3`). Positive: in image pixels. |
| `axis_length` | `[x, y]` lengths of the axes of `oriented_point` and `oriented_circle` markers in image pixels (default `50`). `0` hides the axis. |
| `rendering_script` | JavaScript which draws the marker instead of the default rendering, see below. |

With the default pen, the selected marker is drawn semi-transparent and the marker under the mouse cursor with a dotted line.
Marker handles are always drawn by anno and are not affected by the rendering configuration.

#### Rendering script
`rendering_script` is JavaScript source code, stored as an array of lines.
It can also be edited in the application: select the marker type and open the *Definition* tab, field *Rendering Script*.
The script runs every time the marker is drawn and replaces the default rendering; call `RenderBase()` to draw the default graphics as well.
If the script fails, drawing stops at the error and no message is shown.

Example: an `oriented_rect` drawn as a rectangle with an arrow along its x axis and a label which keeps its size when zooming:
```json
"rendering_script": [
    "SetBaseTransform(true, true)",
    "SetDefaultPen()",
    "DrawRect(-1, -1, 2, 2)",
    "DrawLine(0, 0, 1.2, 0)",
    "SetBaseTransformCosmetic(true)",
    "DrawText(5, -5, category_name + ' ' + text, 10)"
]
```

The script starts in image coordinates: pixels, origin in the top left corner of the image, y axis pointing down.
`SetBaseTransform(scale, rotate)` moves the origin to the marker:

| Marker type | Origin | `rotate = true` | `scale = true` |
|---|---|---|---|
| `rect` | first corner `x0 y0` | no effect | the rectangle becomes `(0, 0)`–`(1, 1)` |
| `oriented_rect` | center | x axis along the marker angle | the rectangle becomes `(-1, -1)`–`(1, 1)` |
| `circle` | center | no effect | the circle gets radius `1` |
| `oriented_circle` | center | x axis along the marker angle | no effect |
| `oriented_point` | the point | x axis along the marker angle | no effect |
| `point` | the point | no effect | no effect |
| `polygon`, `polyline`, `tool` | first point | no effect | no effect |

Functions available to the script:

| Function | Description |
|---|---|
| `SetBaseTransform(scale, rotate)` | Marker coordinate system, see the table above. |
| `SetBaseTransformCosmetic(rotate)` | Marker origin (and rotation), units are screen pixels: drawings keep their size when zooming. |
| `ResetTransform()` | Back to image coordinates. |
| `Translate(dx, dy)`, `Rotate(degrees)` | Move or rotate the current coordinate system. |
| `SetDefaultPen()`, `SetDefaultPen(width)` | Pen in the category color, with the marker type `line_width` or the given width. |
| `SetPen(r, g, b, width)` | Solid pen with the given color, components `0`–`255`. |
| `DrawLine(x0, y0, x1, y1)` | Line. |
| `DrawRect(left, top, width, height)` | Rectangle outline. |
| `DrawEllipse(left, top, width, height)` | Ellipse inscribed into the rectangle. |
| `DrawCircle(x, y, radius)` | Circle. |
| `DrawArc(left, top, width, height, start, end)` | Arc of the ellipse inscribed into the rectangle; angles in degrees, `0` is the x axis direction, positive is counter-clockwise. |
| `DrawText(x, y, text, font_size)` | Bold text with its baseline starting at `x, y`; the font size is scaled together with the coordinate system. |
| `RenderBase()` | Default rendering of the marker. |
| `Property(name)` | Geometry value of the marker: `"width"`, `"height"`, `"radius"` or `"angle"` (radians), depending on the marker type. |
| `PropertyDefinition(name)` | Linear transform `{a, b}` of a shared property (see `shared_properties`). |
| `Custom(name)` | Value of a custom property of the marker (see `custom_properties`). |

A width argument follows the `line_width` rule: negative is in screen pixels, positive in the current coordinate units.
All functions are also available with a `p.` prefix (`p.DrawLine(...)`), as used by older projects.

Variables available to the script:

| Variable | Description |
|---|---|
| `category` | Category id of the marker. |
| `category_name` | Category name of the marker. |
| `marker_type` | Marker type name. |
| `text` | Text of the marker. |
| `shared_index` | Shared instance index of the marker. |
| `cols`, `rows` | Width and height of the image in pixels. |

#### Project script
`definitions.project_script` has the same format and runs once every time the image is drawn, before the markers,
in image coordinates (*Project* tab, field *Project Script*).
It can draw overlays with `SetPen` and the drawing functions; functions which refer to a marker do nothing there.
Functions and variables defined in the project script can be used in the rendering scripts.

Example: a crosshair in the image center.
```json
"project_script": [
    "SetPen(255, 255, 0, -1)",
    "DrawLine(cols / 2, 0, cols / 2, rows)",
    "DrawLine(0, rows / 2, cols, rows / 2)"
]
```

### JSON Schema
The complete project file format is described by a [JSON Schema](https://json-schema.org/) (draft-07):
[`schema/anno-project.schema.json`](schema/anno-project.schema.json).
It covers all fields, including the advanced marker type parameters,
and can be used to validate project files or to get autocompletion and inline documentation in an editor.

Validate a project file with Python ([jsonschema](https://pypi.org/project/jsonschema/) package):
```python
import json
from jsonschema import Draft7Validator

schema = json.load(open("schema/anno-project.schema.json"))
project = json.load(open("my_project.anno"))
for error in Draft7Validator(schema).iter_errors(project):
    print("/".join(map(str, error.absolute_path)), error.message)
```

Use it in VS Code, `settings.json`:
```jsonc
{
    "files.associations": { "*.anno": "json" },
    "json.schemas": [
        {
            "fileMatch": ["*.anno"],
            "url": "https://raw.githubusercontent.com/urobots-io/anno/master/schema/anno-project.schema.json"
        }
    ]
}
```

<details>
<summary>Show the schema</summary>

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "https://raw.githubusercontent.com/urobots-io/anno/master/schema/anno-project.schema.json",
  "title": "anno project file",
  "description": "Project file of the anno labeling tool (*.anno). Contains the marker type definitions of the project and the markers placed on every labeled image.",
  "type": "object",
  "required": ["definitions"],
  "additionalProperties": false,
  "properties": {
    "definitions": {
      "$ref": "#/definitions/projectDefinitions"
    },
    "files": {
      "description": "List of all labeled files with their markers. Files without markers are not stored.",
      "type": "array",
      "items": { "$ref": "#/definitions/file" }
    }
  },

  "definitions": {
    "projectDefinitions": {
      "description": "Project definitions.",
      "type": "object",
      "additionalProperties": false,
      "properties": {
        "files_root_dir": {
          "description": "Folder with images: an absolute path, or a path relative to the project file.",
          "type": "string",
          "examples": ["c:/images", "images"]
        },
        "marker_types": {
          "description": "Marker type definitions. Each key is a unique marker type name, referenced by the \"type\" field of the markers.",
          "type": "object",
          "additionalProperties": { "$ref": "#/definitions/markerType" }
        },
        "user_data": {
          "description": "User information associated with the project. anno stores it as is, except for the optional fields listed here, which are used by the \"Evaluate\" function.",
          "type": "object",
          "properties": {
            "evaluate_url": {
              "description": "URL of a REST service which receives the current image and returns markers for it.",
              "type": "string"
            },
            "evaluate_url_args": {
              "description": "Additional fields sent to the evaluate_url service with every request.",
              "type": "object"
            }
          }
        },
        "project_script": {
          "$ref": "#/definitions/script",
          "description": "JavaScript executed every time an image is rendered, before the markers are drawn. Can be used to draw overlays on the image. The image size is available as \"cols\" and \"rows\"."
        },
        "image_script": {
          "$ref": "#/definitions/script",
          "description": "Deprecated name of \"project_script\". Read only if \"project_script\" is absent; anno saves it as \"project_script\".",
          "deprecated": true
        },
        "files_loader": {
          "description": "Reserved for custom image loaders. Stored as is, not used by anno at the moment.",
          "type": "object",
          "properties": {
            "type": { "type": "string" },
            "params": { "type": "object" }
          }
        }
      }
    },

    "markerType": {
      "description": "Definition of a marker type.",
      "type": "object",
      "required": ["value_type", "categories"],
      "additionalProperties": false,
      "properties": {
        "value_type": {
          "description": "Geometry of the marker. Defines the format of the marker \"value\".",
          "type": "string",
          "enum": [
            "circle",
            "oriented_circle",
            "oriented_point",
            "oriented_rect",
            "point",
            "polygon",
            "polyline",
            "rect",
            "tool"
          ]
        },
        "description": {
          "description": "Optional human-readable description of the marker type.",
          "type": "string"
        },
        "name": {
          "description": "Deprecated name of \"description\". Read only if \"description\" is absent.",
          "type": "string",
          "deprecated": true
        },
        "categories": {
          "description": "Categories of the marker type. Markers reference them by id.",
          "type": "array",
          "items": { "$ref": "#/definitions/category" }
        },
        "line_width": {
          "description": "Width of lines used to draw the marker. A negative value is a scale-independent width in screen pixels, a positive value is a width in image pixels.",
          "type": "integer",
          "default": -3
        },
        "rendering_script": {
          "$ref": "#/definitions/script",
          "description": "JavaScript which draws the marker instead of the default rendering."
        },
        "stamp": {
          "description": "If true, new markers of this type are created with one click, using the sizes from \"stamp_parameters\".",
          "type": "boolean",
          "default": false
        },
        "stamp_parameters": {
          "description": "Initial geometry of stamped markers. Keys are geometry properties of the value type: \"width\", \"height\" (rect, oriented_rect), \"radius\" (circle, oriented_circle), \"angle\" (oriented_point, oriented_rect, oriented_circle).",
          "type": "object",
          "propertyNames": { "$ref": "#/definitions/geometryPropertyName" },
          "additionalProperties": { "type": ["number", "null"] }
        },
        "shared": {
          "description": "If true, all markers of this type share the same geometry (equivalent to \"shared_count\": 1).",
          "type": "boolean",
          "default": false
        },
        "shared_count": {
          "description": "Number of shared marker instances. Markers select one of them with \"shared_index\". Overrides \"shared\".",
          "type": "integer",
          "minimum": 0
        },
        "shared_properties": {
          "description": "Geometry properties shared with other markers through a named project-wide value. Keys are geometry properties of the value type (\"width\", \"height\", \"radius\", \"angle\"). A string value is the name of the shared value; an object additionally defines a linear transform: property = a * shared_value + b.",
          "type": "object",
          "propertyNames": { "$ref": "#/definitions/geometryPropertyName" },
          "additionalProperties": {
            "oneOf": [
              { "type": "string" },
              {
                "type": "object",
                "required": ["name"],
                "additionalProperties": false,
                "properties": {
                  "name": { "type": "string", "description": "Name of the shared value." },
                  "a": { "type": "number", "default": 1 },
                  "b": { "type": "number", "default": 0 }
                }
              }
            ]
          }
        },
        "custom_properties": {
          "description": "Additional user-defined properties of the markers. Keys are property names.",
          "type": "object",
          "additionalProperties": { "$ref": "#/definitions/customPropertyDefinition" }
        },
        "filename_filter": {
          "description": "Regular expression(s) matched against the image file name (relative to files_root_dir). The marker type can be used only on matching files. For shared markers, the n-th expression applies to the n-th shared instance; the last expression applies to all remaining ones.",
          "oneOf": [
            { "type": "string" },
            { "type": "array", "items": { "type": "string" } }
          ]
        },
        "axis_length": {
          "description": "Lengths of the x and y axes of oriented_point and oriented_circle markers in image pixels (default 50). 0 hides the axis, a negative value means the default.",
          "type": "array",
          "items": { "type": "integer" },
          "maxItems": 2
        }
      }
    },

    "category": {
      "type": "object",
      "required": ["id", "name"],
      "additionalProperties": false,
      "properties": {
        "id": {
          "description": "Numerical id of the category, unique within the marker type.",
          "type": "integer"
        },
        "name": {
          "description": "Human-readable name of the category.",
          "type": "string"
        },
        "color": {
          "description": "Color of the category: \"#rrggbb\", \"#aarrggbb\" or an SVG color name such as \"red\". If absent, a standard color is chosen by id.",
          "type": "string",
          "examples": ["#ff0000", "red"]
        }
      }
    },

    "customPropertyDefinition": {
      "type": "object",
      "required": ["type"],
      "additionalProperties": false,
      "properties": {
        "type": {
          "description": "Type of the property value.",
          "type": "string",
          "enum": ["int", "double", "string", "bool", "selector"]
        },
        "default": {
          "description": "Default value of the property, or null."
        },
        "cases": {
          "description": "Allowed values of a \"selector\" property.",
          "type": "array",
          "items": { "type": "string" }
        }
      },
      "allOf": [
        {
          "if": { "properties": { "type": { "const": "int" } } },
          "then": { "properties": { "default": { "type": ["integer", "null"] } } }
        },
        {
          "if": { "properties": { "type": { "const": "double" } } },
          "then": { "properties": { "default": { "type": ["number", "null"] } } }
        },
        {
          "if": { "properties": { "type": { "enum": ["string", "selector"] } } },
          "then": { "properties": { "default": { "type": ["string", "null"] } } }
        },
        {
          "if": { "properties": { "type": { "const": "bool" } } },
          "then": { "properties": { "default": { "type": ["boolean", "null"] } } }
        }
      ]
    },

    "file": {
      "description": "A labeled image.",
      "type": "object",
      "required": ["name", "markers"],
      "additionalProperties": false,
      "properties": {
        "name": {
          "description": "Path to the image, relative to files_root_dir.",
          "type": "string"
        },
        "markers": {
          "description": "List of all markers present on the image.",
          "type": "array",
          "items": { "$ref": "#/definitions/marker" }
        }
      }
    },

    "marker": {
      "type": "object",
      "required": ["type", "category", "value"],
      "additionalProperties": false,
      "properties": {
        "type": {
          "description": "Marker type name, a key of definitions.marker_types.",
          "type": "string"
        },
        "category": {
          "description": "Category id, one of the categories of the marker type.",
          "type": "integer"
        },
        "value": {
          "$ref": "#/definitions/markerValue"
        },
        "children": {
          "description": "Additional values of the marker. Used by polygons: every child is one more contour (e.g. a hole).",
          "type": "array",
          "items": {
            "type": "object",
            "required": ["value"],
            "additionalProperties": false,
            "properties": {
              "value": { "$ref": "#/definitions/markerValue" }
            }
          }
        },
        "text": {
          "description": "Optional free text attached to the marker.",
          "type": "string"
        },
        "shared_index": {
          "description": "Index of the shared marker instance (only for shared marker types).",
          "type": "integer",
          "minimum": 0,
          "default": 0
        },
        "custom_properties": {
          "description": "Values of the custom properties defined by the marker type.",
          "type": "object",
          "additionalProperties": { "type": ["string", "number", "boolean", "null"] }
        }
      }
    },

    "markerValue": {
      "description": "Marker geometry as space-separated numbers. All coordinates are in image pixels, angles are in radians. Format by value type:\n- circle: center_x center_y radius\n- oriented_circle: center_x center_y angle radius\n- oriented_point: x y angle\n- oriented_rect: center_x center_y width height angle\n- point: x y\n- polygon: x0 y0 x1 y1 ... xN yN\n- polyline, tool: x0 y0 x1 y1 ... xN yN\n- rect: x0 y0 x1 y1",
      "type": "string",
      "pattern": "^\\s*[-+]?(\\d+\\.?\\d*|\\.\\d+)([eE][-+]?\\d+)?(\\s+[-+]?(\\d+\\.?\\d*|\\.\\d+)([eE][-+]?\\d+)?)*\\s*$",
      "examples": ["789.886 627.372 146.942 93.1009 -0.486899"]
    },

    "script": {
      "description": "JavaScript source code, one array element per line.",
      "type": "array",
      "items": { "type": "string" }
    },

    "geometryPropertyName": {
      "type": "string",
      "enum": ["width", "height", "radius", "angle"]
    }
  }
}
```

</details>
