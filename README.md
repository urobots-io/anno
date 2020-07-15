# anno
Cross-platform image labeling tool for AI

Project file format:
```jsonc
{
    /** Project definitions.
    */
    "definitions": {
        /** Folder with images, absolute path or relative path to the project file.
        */
        "files_root_dir": "c:/images",
        /** Marker types definitions.
        */
        "marker_types": {
            /** Key represents a unique marker type name.
            */
            "my_marker_type": {

            }
        },
        /** Optional object, can be used to store user information accosiated with
            the project. Anno does not process or modify it.
        */
        "user_data": {
        }
    },

    /** List of all labeled files with their markers.
    */
    "files": [
        {
            /** List of all markers present on the image.
            */
            "markers": [
                {
                    /** Marker category.
                    */
                    "category": 0,
                    /** Marker type name.
                    */
                    "type": "my_marker_type",
                    /** Marker data.
                    */
                    "value": "789.886 627.372 146.942 93.1009 -0.486899"
                }
            ],
            /** Path to the image, relative to the files_root_dir.
            */
            "name": "image.png"
        }
    ]
}
```