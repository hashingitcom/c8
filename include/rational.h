/*
 * rational.h
 */
#ifndef __C8_RATIONAL_H
#define __C8_RATIONAL_H

#include <integer.h>

namespace c8 {
    enum class rational_comparison {
        lt,                                 // Less than
        eq,                                 // Equal
        gt                                  // Greater than
    };

    class rational {
    public:
        /*
         * Constructors.
         */
        rational() {
            num_ = 0;
            denom_ = 1;
        }

        rational(long long n, unsigned long long d) : num_(n), denom_(d) {
            normalize();
        }

        rational(integer v) : num_(v) {
            denom_ = 1;
        }

        rational(const std::string &v);
        rational(const rational &v) = default;
        rational(rational &&v) = default;
        ~rational() = default;
        auto operator =(const rational &v) -> rational & = default;
        auto operator =(rational &&v) -> rational & = default;

        auto add(const rational &v) const -> rational;
        auto subtract(const rational &v) const -> rational;
        auto multiply(const rational &v) const -> rational;
        auto divide(const rational &v) const -> rational;
        auto compare(const rational &v) const -> rational_comparison;
        friend auto operator<<(std::ostream &outstr, const rational &v) -> std::ostream &;

        auto operator+(const rational &v) const -> rational {
            return add(v);
        }

        auto operator+=(const rational &v) {
            *this = add(v);
        }

        auto operator-(const rational &v) const -> rational {
            return subtract(v);
        }

        auto operator-() const -> rational {
            rational res = *this;
            res.num_ = -num_;
            res.denom_ = denom_;
            return res;
        }

        auto operator-=(const rational &v) {
            *this = subtract(v);
        }

        auto operator*(const rational &v) const -> rational {
            return multiply(v);
        }

        auto operator*=(const rational &v) {
            *this = multiply(v);
        }

        auto operator/(const rational &v) const -> rational {
            return divide(v);
        }

        auto operator/=(const rational &v) {
            *this = divide(v);
        }

        auto operator==(const rational &v) const -> bool {
            return compare(v) == rational_comparison::eq;
        }

        auto operator!=(const rational &v) const -> bool {
            return compare(v) != rational_comparison::eq;
        }

        auto operator>(const rational &v) const -> bool {
            return compare(v) == rational_comparison::gt;
        }

        auto operator>=(const rational &v) const -> bool {
            return compare(v) != rational_comparison::lt;
        }

        auto operator<(const rational &v) const -> bool {
            return compare(v) == rational_comparison::lt;
        }

        auto operator<=(const rational &v) const -> bool {
            return compare(v) != rational_comparison::gt;
        }

    private:
        integer num_;                       // Numerator
        natural denom_;                     // Denominator

        auto normalize() -> void;
    };
}

#endif /* __C8_RATIONAL_H */

