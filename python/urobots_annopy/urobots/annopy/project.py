"""
Anno project file.

"""
import json
import os

from . import value as val

MARKER_TYPES_TO_VALUES =  {
    'rect': val.RectValue,
    'point': val.PointValue,
    'oriented_point': val.OrientedPointValue,
    'oriented_rect': val.OrientedRectValue,
    'circle': val.CircleValue,
    'polygon': val.PolygonValue
}

class Project:
    """
    Parsed data of an anno project.
    """
    def __init__(self, file_name):
        self._file_name = file_name
        with open(file_name) as f:
            self._data = json.load(f)

        self._definitions = self._data['definitions']
        self._user_data = self._definitions.get('user_data', None)
        self._files = self._data['files']

        self._files_root_dir_abs = self._definitions['files_root_dir']
        if not os.path.isabs(self._files_root_dir_abs):
            self._files_root_dir_abs = os.path.dirname(self._file_name) + '/' + self._files_root_dir_abs

    def save(self, file_name=None):
        file_name = self._file_name if file_name is None else file_name
        with open(file_name, 'w') as f:
            json.dump(self._data, f, sort_keys=True, indent=4)
        self._file_name = file_name

    @property
    def data(self):
        """
        The whole data dictionary from json.
        """
        return self._data

    @property
    def file_name(self):
        return self._file_name

    @property
    def files_root_dir_abs(self):
        """
        :return: Absolute path to the files root dir.
        """
        return self._files_root_dir_abs

    @property
    def definitions(self):
        """
        A shortcut to the definitions section.
        """
        return self._definitions

    @property
    def user_data(self):
        """
        A shortcut to the user_data section.
        """
        return self._user_data

    @property
    def files(self):
        """
        A shortcut to the files section.
        """
        return self._files

    def get_full_image_file_name(self, file_name):
        return os.path.abspath(os.path.join(self._files_root_dir_abs, file_name))

    def get_custom_property_default(self, marker_type, prop, default=None):
        """
        Get the default value of a custom property.
        :param marker_type: type of the marker.
        :param prop: property name.
        :param default: if this custom property is not defined in anno, returns this value.
        If it is None, throw an exception.
        :return: default value.
        """
        marker_type_def = self._definitions['marker_types'][marker_type]
        if 'custom_properties' in marker_type_def:
            if prop in marker_type_def['custom_properties']:
                return marker_type_def['custom_properties'][prop]['default']
        if default is not None:
            return default
        raise ValueError(f"No custom property {prop} for marker type {marker_type} in anno file {self.file_name}")

    def get_custom_property(self, marker, prop, default=None):
        """
        Get the value of a custom property in a given marker.
        :param marker: marker object.
        :param prop: property name
        :param default: if this custom property is not defined in anno, returns this value.
        If it is None, throw an exception.
        :return: property value from the marker or the default value.
        """
        if 'custom_properties' in marker:
            cp = marker['custom_properties']
            if prop in cp:
                return cp[prop]
        # Evaluate default value only once here for speed-optimization.
        return self.get_custom_property_default(marker['type'], prop, default)

    def create_value(self, marker):
        """ Creates a value object from a marker, based on the marker definition.
        """
        marker_type = marker['type']
        marker_definition = self._definitions['marker_types'][marker_type]
        ctor = MARKER_TYPES_TO_VALUES[marker_definition['value_type']]
        value = ctor(marker['value'])
        return value



def make_marker(marker_type, category, value):
    """
    Makes anno marker objects.
    """
    m = {
        "category": category,
        "type": marker_type,
    }
    if isinstance(value, val.Value):
        value.write_to_marker(m)
    else:
        m["value"] = value

    return m

def make_file(name, markers):
    """
    Makes anno file object.
    """
    return {
        "markers": markers,
        "name": name
    }


