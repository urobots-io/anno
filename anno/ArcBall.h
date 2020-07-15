#pragma once
#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <math.h>

// ArcBall original (java) source:
// https://gist.github.com/vilmosioo/5318327

struct ArcBall {
    QVector3D StVec; // Saved click vector
    QVector3D EnVec; // Saved drag vector
    float adjustWidth; // Mouse bounds width
    float adjustHeight; // Mouse bounds height

    ArcBall(float NewWidth, float NewHeight) {
        setBounds(NewWidth, NewHeight);
    }

    QVector3D mapToSphere(QVector2D point) {
        // Copy paramter into temp point
        QVector2D tempPoint = point;

        // Adjust point coords and scale down to range of [-1 ... 1]
        tempPoint.setX((point.x() * adjustWidth) - 1.0f);
        tempPoint.setY(1.0f - (point.y() * adjustHeight));

        // Compute the square of the length of the vector to the point from the
        // center
        float length = (tempPoint.x() * tempPoint.x()) + (tempPoint.y() * tempPoint.y());

        QVector3D vector;

        // If the point is mapped outside of the sphere... (length > radius
        // squared)
        if (length > 1.0f) {
            // Compute a normalizing factor (radius / sqrt(length))
            float norm = (float)(1.0 / sqrt(length));

            // Return the "normalized" vector, a point on the sphere
            vector.setX(tempPoint.x() * norm);
            vector.setY(tempPoint.y() * norm);
            vector.setZ(0.0f);
        }
        else // Else it's on the inside
        {
            // Return a vector to a point mapped inside the sphere sqrt(radius
            // squared - length)
            vector.setX(tempPoint.x());
            vector.setY(tempPoint.y());
            vector.setZ((float)sqrt(1.0f - length));
        }

        return vector;
    }

    void setBounds(float NewWidth, float NewHeight) {
        // Set adjustment factor for width/height
        adjustWidth = 1.0f / ((NewWidth - 1.0f) * 0.5f);
        adjustHeight = 1.0f / ((NewHeight - 1.0f) * 0.5f);
    }

    // Mouse down
    void click(QVector2D NewPt) {
        StVec = mapToSphere(NewPt);
    }

    // Mouse drag, calculate rotation
    QQuaternion drag(QVector2D NewPt) {
        // Map the point to the sphere
        EnVec = mapToSphere(NewPt);

        // Return the quaternion equivalent to the rotation
        QQuaternion NewRot;
        // Compute the vector perpendicular to the begin and end vectors
        QVector3D Perp = QVector3D::crossProduct(StVec, EnVec);

        // Compute the length of the perpendicular vector
        if (Perp.length() > 0.0001f) // if its non-zero
        {
            // We're ok, so return the perpendicular vector as the transform
            // after all
            NewRot.setX(Perp.x());
            NewRot.setY(Perp.y());
            NewRot.setZ(Perp.z());
            // In the quaternion values, w is cosine (theta / 2), where
            // theta is rotation angle
            NewRot.setScalar(QVector3D::dotProduct(StVec, EnVec));
        }
        else // if its zero
        {
            // The begin and end vectors coincide, so return an identity
            // transform
            NewRot.setX(0);
            NewRot.setY(0);
            NewRot.setZ(0);
            NewRot.setScalar(0);
        }
        return NewRot;
    }
};

