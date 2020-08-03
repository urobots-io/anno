# anno
Cross-platform tool for labeling image data.
[Video introduction to anno](https://youtu.be/RrjGU2vSIOM).

<img src="https://user-images.githubusercontent.com/9085494/89163912-a8055380-d576-11ea-8f8d-da664c6f4687.png"/>

## Features
- Supports a wide range of marker types:
  - Point
  - Oriented point
  - Axis aligned rectangle
  - Rectangle with rotation
  - Polygon
  - Polyline
- Configurable rendering of all marker types.
- Configurable dependencies between properties of markers: i.e. some markers can share width or height or position on image.
- Supports a wide variety of image formats. Pre-build Windows version supports EXR format.
- The project file is JSON file wich contains project-specific marker definitions and labeling information.
- The user interface is optimized for fast sub-pixel accurate labeling.

<img src="https://user-images.githubusercontent.com/9085494/89164042-d5520180-d576-11ea-8a74-f4a13a6716d7.gif"/>

## Requirements
- Windows / macOS / Ubuntu

## Installation
There are options:
- Pre-build binaries from [the release section](https://github.com/urobots-io/anno/releases).
- Cross-platform build is possible with Qt Creator from the open source [Qt setup](https://www.qt.io/).

## Specifications
Project file format:
```jsonc
{
    /** Project definitions. */
    "definitions": {
        /** Folder with images, absolute path or relative path to the project file. */
        "files_root_dir": "c:/images",
        /** A dictionary with marker types definitions. */
        "marker_types": {
            /** Key represents a unique marker type name. */
            "my_marker_type": {
                /** Optional description of the marker type. */
                "description": "use carefully",
                /** Type of the marker. Following types are supported:
                    - circle
                    - oriented_point
                    - oriented_rect    
                    - point
                    - polygon
                    - polyline
                    - rect
                */
                "value_type": "rect",
                /** List of categories */ 
                "categories": [
                    {
                        /** Numerical id of the category */
                        "id": 0,
                        /** Human name of the category */
                        "name": "Ok",
                        /** Color of the category */
                        "color": "red"
                    }
                ],
                
                /** Optional width of lines used to draw the marker. 
                    Negative value defines scale independed width in screen pixels,
                    positive value defines width in image pixels */
                "line_width": -3
                
                /** Advanced parameters: stamp, stamp_parameters, axis_length, 
                    rendering_script, shared_count, 
                    filename_filter, shared_properties, 
                    custom_properties
                */
            }
        },
        /** Optional object, can be used to store user information accosiated with
            the project. Anno does not process or modify it. */
        "user_data": {
        }
    },

    /** List of all labeled files with their markers. */
    "files": [
        {
            /** Path to the image, relative to the files_root_dir. */
            "name": "image.png"
            /** List of all markers present on the image. */
            "markers": [
                {
                    /** Marker category. */
                    "category": 0,
                    /** Marker type name. */
                    "type": "my_marker_type",
                    /** Marker data. */
                    "value": "789.886 627.372 146.942 93.1009 -0.486899"
                }
            ]
        }
    ]
}
```

The format of the marker data depends on the type of marker value:
* circle: ```center_x center_y radius```
* oriented_point: ```x y angle```
* oriented_rect: ```center_x center_y width height angle```
* point: ```x y```
* polygon: ```x0 y0 x1 y1 ... xN yN```
* polyline: ```x0 y0 x1 y1 ... xN yN```
* rect: ```x0 y0 x1 y1```

All coordinates are in image pixels, angles are in radians.
