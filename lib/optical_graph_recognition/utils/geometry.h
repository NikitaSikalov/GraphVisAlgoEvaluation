#pragma once

#include <exception>

namespace ogr::utils {
    struct PlanarVector {
        double x = 0;
        double y = 0;

        void Normalize() {
            constexpr double kEps = 1e-6;
            const double module = Abs();
            if (module <= kEps) {
                throw std::runtime_error{"Invalid normalize operation over 0 vector"};
            }

            x /= module;
            y /= module;
        }

        double GetAngle() {
            const double angle = atan(fabs(y) / fabs(x));
            assert(angle >= 0);

            if (y >= 0 && x >= 0) {
                return angle;
            }

            if (y >= 0 && x <= 0) {
                return M_PI - angle;
            }

            if (y <= 0 && x <= 0) {
                return M_PI + angle;
            }

            return 2 * M_PI - angle;
        }

        double Abs() noexcept {
            return sqrt(x * x + y * y);
        }
    };

    inline double Rad2Deg(double rad) {
        return rad * 180.0 / M_PI;
    }

    inline double NormalizeAngle(const double angle) {
        assert(angle >= -360 && angle <= 720);

        if (angle < 0) {
            return 360 + angle;
        }

        if (angle >= 360) {
            return angle - 360;
        }

        return angle;
    }

    inline double AbsDiffAngles(double angle1, double angle2) {
        angle1 = NormalizeAngle(angle1);
        angle2 = NormalizeAngle(angle2);
        const double max = std::max(angle1, angle2);
        const double min = std::min(angle1, angle2);
        assert(max >= 0 && max < 360);
        assert(min >= 0 && min < 360);

        const double result = max - min <= min + 360 - max ? max - min : min + 360 - max;
        assert(result >= 0 && result < 180);

        return result;
    }

    inline double AlignAngle(const double angle, const double sample) {
        assert(angle >= 0 && angle < 360);
        assert(sample >= 0 && sample <= 360);

        if (fabs(sample - angle) <= 180) {
            return angle;
        }

        if (sample < 180) {
            return angle - 360;
        }

        assert(angle < 180);
        return 360 + angle;
    }
}