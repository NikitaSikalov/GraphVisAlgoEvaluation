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
}