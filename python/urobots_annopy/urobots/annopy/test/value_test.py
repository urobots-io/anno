from pytest import approx
from datetime import datetime

import numpy as np

from urobots.annopy.value import *


class ValueTest:
    def test_angle(self):
        angles = [0, 0.5, -0.5, 1, 1.25]
        for angle in angles:
            value = OrientedPointValue([1, 1, angle])
            assert abs(angle - value.angle) < 1e-5

    def test_image_self_transform(self):
        # Use oriented point as a concrete class because it containts both rotation and translation
        tx = 10
        ty = 20
        a = 0.5
        value = OrientedPointValue([tx, ty, a])
        # Points in own cs.
        my_points = np.array([[0, 0], [0, 1], [1, 0], [1, 1]], dtype=np.float32)
        image_points = value.to_image(my_points)
        for i in range(len(my_points)):
            mp = my_points[i]
            im = np.array([mp[0] * np.cos(a) - mp[1] * np.sin(a) + tx,
                           mp[0] * np.sin(a) + mp[1] * np.cos(a) + ty])
            assert np.allclose(im, image_points[i])

        my_points1 = value.from_image(image_points)
        assert np.allclose(my_points, my_points1)


class RectValueTest:
    def test_tostring(self):
        # (value, pos_precision)
        data = [
            ("0 0 1 1", 0),
            ("0.123 0.456 1.589 1.978", 3)
            ]
        for d in data:
            value = RectValue(d[0])
            string = value.tostring(pos_precision=d[1])
            assert d[0] == string


class PointValueTest:
    def test_tostring(self):
        # (value, pos_precision)
        data = [
            ("1 2", 0),
            ("0.123 0.456", 3)
            ]
        for d in data:
            value = PointValue(d[0])
            string = value.tostring(pos_precision=d[1])
            assert d[0] == string

class OrientedPointValueTest:
    def test_tostring(self):
        # (value, pos_precision, angle_precision)
        data = [
            ("1 2 3", 0, 0),
            ("0.123 0.456 1.589789", 3, 6)
            ]
        for d in data:
            value = OrientedPointValue(d[0])
            string = value.tostring(pos_precision=d[1], angle_precision=d[2])
            assert d[0] == string


class OrientedRectValueTest:
    def test_tostring(self):
        # (value, pos_precision, angle_precision)
        data = [
            ("1 2 4 5 3", 0, 0),
            ("0.123 0.456 2.001 0.020 1.589789", 3, 6)
            ]
        for d in data:
            value = OrientedRectValue(d[0])
            string = value.tostring(pos_precision=d[1], angle_precision=d[2])
            assert d[0] == string

class CircleValueTest:
    def test_tostring(self):
        # (value, pos_precision)
        data = [
            ("0 1 2", 0),
            ("0.123 0.456 1.589", 3)
            ]
        for d in data:
            value = CircleValue(d[0])
            string = value.tostring(pos_precision=d[1])
            assert d[0] == string


class PolygonValueTest:
    _show_images = False

    def test_fill(self):
        value = [100, 50, 50, 90, 0, 50, 50, 0]
        children = [
            {"value" : [60, 60, 40, 60, 40, 40, 60, 40]}
        ]
        poly = PolygonValue(value, children)
        image = np.zeros((100, 100), dtype=np.float32)
        poly.fill(image, (1, 1, 1))
        if __class__._show_images:
            cv2.imshow("image", image)
            cv2.waitKey(1)

    def test_is_point_inside(self):
        value = [100, 50, 50, 90, 0, 50, 50, 0]
        children = [
            {"value" : [60, 60, 40, 60, 40, 40, 60, 40]}
        ]
        poly = PolygonValue(value, children)

        assert not poly.is_point_inside((-10, -10))
        assert not poly.is_point_inside((0, 0))
        assert not poly.is_point_inside((50, 50))
        assert not poly.is_point_inside((50, 91))
        assert not poly.is_point_inside((50, 90))

        assert poly.is_point_inside((50, 10))
        assert poly.is_point_inside((39, 39))
        assert poly.is_point_inside((39, 50))
        assert poly.is_point_inside((50, 89.5))

        start_time = datetime.now()
        r = 0
        repetitions = 10000
        for i in range(0, 10000):
            r += poly.is_point_inside((38.5, 38.5))
        calls_per_second = repetitions / (datetime.now() - start_time).total_seconds()
        print(calls_per_second)

    def test_distance_to_point(self):
        value = [100, 50, 50, 90, 0, 50, 50, 0]
        children = [
            {"value": [60, 60, 40, 60, 40, 40, 60, 40]}
        ]
        poly = PolygonValue(value, children)

        assert poly.distance_to_point((0, 0)) == approx(-50/np.sqrt(2))
        assert poly.distance_to_point((0, 50)) == approx(0)
        assert poly.distance_to_point((50, 90)) == approx(0)
        assert poly.distance_to_point((40, 40)) == approx(0)
        assert poly.distance_to_point((50, 50)) == approx(-10)
        assert poly.distance_to_point((50, 44)) == approx(-4)
        assert poly.distance_to_point((50, 39)) == approx(1)
        assert poly.distance_to_point((50, 20)) == approx(20 / np.sqrt(2))
        assert poly.distance_to_point((50, 19)) == approx(19 / np.sqrt(2))

        start_time = datetime.now()
        r = 0
        repetitions = 10000
        for i in range(0, 10000):
            r += poly.distance_to_point((38.5, 38.5))
        calls_per_second = repetitions / (datetime.now() - start_time).total_seconds()
        print(calls_per_second)

    def test_tostring(self):
        # (value, children, pos_precision)
        data = [
            ("0 0 1 0 1 1 0 1", None, 0),
            ("0.123 0.456 3.589 0.978 1.121 4.123", None, 3),
            ("1.23 0.12 3.59 1.78 1.21 4.23", [
                "0.11 1.22 3.78 2.64 1.01 4.53",
                "1.27 2.12 4.59 2.78 2.31 2.23"
            ], 2)
            ]
        for d in data:
            value = PolygonValue(d[0], d[1])
            string = value.tostring(pos_precision=d[2])
            assert type(string) == list
            assert d[0] ==  string[0]
            if d[1] is None:
                assert len(string) == approx(1)
            else:
                assert string[1:] == d[1]
