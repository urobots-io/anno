"""
Anno values.

General conventions:

- The original marker value from the textual representation is converted to an internal format convenient for algorithms.
  The reverse conversion is done by tostring() method. It will return an equivalent representation, which is may differ from
  the original one.
- For all markers point test does not include marker's border. For example, a circle with radius of 0 has no points inside.

"""

import abc

import cv2
import numpy as np


class Value:
    """
    Base class for marker value.

    Derived classes must have the following members:
    _image_pose_self: pose of the value.
    """
    def __init__(self):
        self._a = 0

    @staticmethod
    def _get_value_as_list(value, expected_length = None):
        if type(value) == str:
            value = [float(s) for s in value.split()]
        if expected_length is not None and len(value) != expected_length:
            raise ValueError("Wrong number of values {}, expected {}.".format(len(value), expected_length))
        return value

    def _make_pose(self, ox, oy, angle):
        ca = np.cos(angle)
        sa = np.sin(angle)
        self._image_pose_self = np.array([[ca, -sa, ox], [sa, ca, oy], [0, 0, 1]], dtype=np.float32)

    @property
    def image_pose_self(self):
        return self._image_pose_self

    @property
    def o(self):
        """ Origin of the coordinate system. """
        return self._image_pose_self[:2, 2:].squeeze()

    def transform(self, t):
        """
        Transform the pose by the matrix t. In other words, the marker is "moved" by the matrix t.
        :param t: 3x3 2d homogenous transformation matrix.
        """
        self._image_pose_self = np.dot(t, self._image_pose_self)

    @property
    def a(self):
        """ Angle """
        return self._a

    @a.setter
    def a(self, value):
        self._a = value
        self._make_pose(self.x, self.y, self._a)

    @property
    def ax(self):
        """ X-axis of the coordinate system. """
        return self._image_pose_self[:2, 0:1].squeeze()

    @property
    def ay(self):
        """ Y-axis of the coordinate system. """
        return self._image_pose_self[:2, 1:2].squeeze()

    @property
    def x(self):
        """ X coordinate of the origin. """
        return self._image_pose_self[0, 2]

    @x.setter
    def x(self, value):
        self._image_pose_self[0, 2] = value

    @property
    def y(self):
        """ Y coordinate of the origin. """
        return self._image_pose_self[1, 2]

    @y.setter
    def y(self, value):
        self._image_pose_self[1, 2] = value

    @property
    def angle(self):
        return np.arctan2(self._image_pose_self[1, 0], self._image_pose_self[0, 0])

    @abc.abstractmethod
    def tostring(self, pos_precision=2, angle_precision=4):
        """
        Convert to textual representation in anno file format.

        :param pos_precision: precision for positional parameters (x, y, sizes, etc.)
        :param angle_precision: precision for angular parameters.
        :return:
        """
        return ''

    def write_to_marker(self, marker, pos_precision=2, angle_precision=4):
        """
        Writes itself to an anno marker object.

        :param marker: marker object.
        """
        marker["value"] = self.tostring(pos_precision, angle_precision)

    def __str__(self):
        return self.tostring()

    def _transform_points(self, t, points):
        """
        Apply a transform on points.
        :param t: a 3x3 homegenous 2d transform matrix.
        :param points: a point or an array of points in rows.
        :return: a point or an array of transformed points in rows.
        """
        p = np.array(points, dtype=np.float32).reshape(-1, 2)
        p = np.concatenate((p, np.ones((p.shape[0], 1), dtype=np.float32)), axis=1)
        tp = np.dot(p, t.T)  # Change order and transpose because the points are not in columns.
        tp = tp[:, :2].reshape(points.shape)
        return tp

    def from_image(self, image_point):
        """
        Transform image point to the coordinate system of the self.

        :param image_point point in image coordinates
        :return point in self coordinate system.
        """
        return self._transform_points(np.linalg.inv(self._image_pose_self), image_point)

    def to_image(self, self_point):
        """
        Transform point(s) in own coordinate system to a point in the image coordinate system.

        :param self_point point or array of points in rows in own coordinate system.
        :return point or array of points in rows in image coordinate system.
        """
        return self._transform_points(self._image_pose_self, self_point)

    def distance_from_o(self, point):
        """
        Computes distance between point and origin.

        :param point point in image coordinates.
        :return unsigned distance from origin.
        """
        d = np.linalg.norm(point - self.o)
        return d

class RectValue(Value):
    """ Rect value.
        Origin is at the top left corner.
    """
    def __init__(self, value):
        Value.__init__(self)
        value = Value._get_value_as_list(value, 4)
        self._make_pose(
            min(value[0], value[2]),
            min(value[1], value[3]),
            0)
        self._sx = abs(value[2] - value[0])
        self._sy = abs(value[3] - value[1])

    @property
    def sx(self):
        """ X-size (width)."""
        return self._sx

    @sx.setter
    def sx(self, value):
        self._sx = value

    @property
    def sy(self):
        """ Y-size (height)."""
        return self._sy

    @sy.setter
    def sy(self, value):
        self._sy = value

    @property
    def br(self):
        """ Bottom-right corner (opposite to the origin)."""
        return self.o + np.array([self._sx, self._sy])

    def contains(self, point):
        """
        Checks if a point is inside the marker.

        :param point point in image coordinates
        :return True, if the point is inside the marker.
        """
        p = self.from_image(point)
        result = 0 < p[0] and p[0] < self._sx and 0 < p[1] and p[1] < self._sy
        return result

    def tostring(self, pos_precision=2, angle_precision=4):
        fmt_pos = '{:0.' + str(pos_precision) + 'f}'
        return " ".join(map(lambda x: fmt_pos.format(x), [self.x, self.y, self.br[0], self.br[1]]))


class PointValue(Value):
    """ Point value. """
    def __init__(self, value):
        Value.__init__(self)
        value = Value._get_value_as_list(value, 2)
        self._make_pose(value[0], value[1], 0)

    def tostring(self, pos_precision=2, angle_precision=None):
        """
        Write marker to string.

        :param pos_precision: position precision.
        :param angle_precision: not used.
        :return:
        """
        fmt_pos = '{:0.' + str(pos_precision) + 'f}'
        return " ".join(map(lambda x: fmt_pos.format(x), [self.x, self.y]))

class OrientedPointValue(Value):
    """ OrientedPoint value. """
    def __init__(self, value):
        Value.__init__(self)
        value = Value._get_value_as_list(value, 3)
        self._a = value[2]
        self._make_pose(value[0], value[1], value[2])

    def tostring(self, pos_precision=2, angle_precision=4):
        fmt_pos = '{:0.' + str(pos_precision) + 'f}'
        fmt_angle = '{:0.' + str(angle_precision) + 'f}'
        return " ".join(map(lambda x: fmt_pos.format(x), [self.x, self.y])) + " " + fmt_angle.format(self.angle)


class OrientedRectValue(Value):
    """ OrientedRect value.
        It's origin is in the center of the rectangle.
    """
    def __init__(self, value):
        """
        Create object.
        :param value:  [origin_x, origin_y, size_x, size_y, angle]
        """
        Value.__init__(self)
        value = Value._get_value_as_list(value, 5)
        self._a = value[4]
        self._make_pose(value[0], value[1], value[4])
        self._sx = value[2]
        self._sy = value[3]

    @property
    def sx(self):
        """ X-size (width) of the rectangle. """
        return self._sx

    @sx.setter
    def sx(self, value):
        self._sx = value

    @property
    def sy(self):
        """ Y-size, (height) of the rectangle. """
        return self._sy

    @sy.setter
    def sy(self, value):
        self._sy = value

    def contains(self, point):
        """
        Checks if a point is inside the marker.

        :param point point in image coordinates
        :return True, if the point is inside the marker.
        """
        p = np.abs(self.from_image(point) * 2)
        result = p[0] < self._sx and p[1] < self._sy
        return result

    def tostring(self, pos_precision=2, angle_precision=4):
        fmt_pos = '{:0.' + str(pos_precision) + 'f}'
        fmt_angle = '{:0.' + str(angle_precision) + 'f}'
        return " ".join(map(lambda x: fmt_pos.format(x), [self.x, self.y, self.sx, self.sy])) + \
               " " + fmt_angle.format(self.angle)


class CircleValue(Value):
    """ OrientedRect value. """
    def __init__(self, value):
        Value.__init__(self)
        value = Value._get_value_as_list(value, 3)
        self._make_pose(value[0], value[1], 0)
        self._r = value[2]

    @property
    def r(self):
        """ Radius)."""
        return self._r

    @r.setter
    def r(self, value):
        self._r = value

    def contains(self, point):
        """
        Checks if a point is inside the marker.

        :param point point in image coordinates
        :return True, if the point is inside the marker.
        """
        d = self.distance_from_o(point)
        return d < self._r

    def tostring(self, pos_precision=2, angle_precision=4):
        fmt_pos = '{:0.' + str(pos_precision) + 'f}'
        return " ".join(map(lambda x: fmt_pos.format(x), [self.x, self.y, self._r]))


class PolygonValue(Value):
    """ Polygon value. """
    def __init__(self, value, children=None):
        """
        Create a new instance.

        :param value: a string or a list of floats for outer contour.
        :param children: a list of children (inner contours aka holes):
         - strings
         - lists of floats
         - dictionaries as in anno files: {'value': value}

        Internal representation is a list or np.arrays [*, 2] self._poly.
        self._poly[0] is the outer contour. The optional rest are children (holes).
        """
        Value.__init__(self)


        self._poly = [np.array(Value._get_value_as_list(value), dtype=np.float32).reshape(-1, 2)]
        if children is not None:
            for child in children:
                if type(child) == dict:
                    child_value = child["value"]
                else:
                    child_value = child
                child_value = np.array(Value._get_value_as_list(child_value), dtype=np.float32).reshape(-1, 2)
                self._poly.append(child_value)

        self._bounds = np.array([
            np.min(self._poly[0], axis=0),
            np.max(self._poly[0], axis=0)],
            dtype=np.float32)

        self._make_pose(0, 0, 0)

    @property
    def bounds(self):
        """
        Gets bounds of the polygon.

        :return: numpy array [[min_x, min_y], [max_x, max_y]].
        """
        return self._bounds

    def fill(self, image, color):
        int_poly = []
        for p in self._poly:
            int_poly.append(np.round(p).astype(np.int32))
        cv2.fillPoly(image, int_poly, color=color)

    def contains(self, point):
        """
        Checks if a point is inside the marker.

        :param point point in image coordinates
        :return True, if the point is inside the marker.
        """
        point = tuple(point)
        if point[0] <= self._bounds[0][0] or point[1] <= self._bounds[0][1]:
            return False
        if point[0] >= self._bounds[1][0] or point[1] >= self._bounds[1][1]:
            return False
        if cv2.pointPolygonTest(self._poly[0], point, measureDist=False) <= 0:
            return False
        for hole in self._poly[1:]:
            if cv2.pointPolygonTest(hole, point, measureDist=False) >= 0:
                return False
        return True


    def distance_to_point(self, point):
        """
        Measures distance from the border to the point.

        :param point point in image coordinates
        :return distance. Positive is inside the polygon, negative - outside.
        """
        point = tuple(point)
        dist = cv2.pointPolygonTest(self._poly[0], point, measureDist=True)
        if dist <= 0:
            # Point is outside of outer polygon
            return dist

        for hole in self._poly[1:]:
            inner_dist = cv2.pointPolygonTest(hole, point, measureDist=True)
            if inner_dist >= 0:
                # Point is in the hole
                return -inner_dist
            dist = min(dist, -inner_dist)

        return dist

    def tostring(self, pos_precision=2):
        """
        Convert to string representation.

        :return: Returns a list of strings, the first element corresponds to the outer contour,
        the rest to the children.
        """
        fmt_pos = '{:0.' + str(pos_precision) + 'f}'
        def points_to_string(points):
            return " ".join(map(lambda x: fmt_pos.format(x), list(points.ravel())))

        strings = list(map(points_to_string, self._poly))

        return strings

    def write_to_marker(self, marker, pos_precision=2, angle_precision=4):
        text = self.tostring(pos_precision, angle_precision)
        marker["value"] = text[0]
        if len(text) > 0:
            marker["children"] = list(map(lambda x: {"value": x}, text[1:]))

    def __str__(self):
        return str(self.tostring())


