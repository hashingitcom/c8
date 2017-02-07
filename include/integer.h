/*
 * integer.h
 */
#ifndef __C8_INTEGER_H
#define __C8_INTEGER_H

#include <natural.h>

namespace c8 {
    enum class integer_comparison {
        lt,                                 // Less than
        eq,                                 // Equal
        gt                                  // Greater than
    };

    class integer {
    public:
        /*
         * Constructors.
         */
        integer() {
            negative_ = false;
        }

        integer(long long v) : magnitude_((v >= 0) ? static_cast<unsigned long long>(v) : static_cast<unsigned long long>(-v)) {
            negative_ = (v < 0) ? true : false;
        }

        integer(natural v) : magnitude_(v) {
            negative_ = false;
        }

        integer(const std::string &v);
        integer(const integer &v) = default;
        integer(integer &&v) = default;
        ~integer() = default;
        auto operator =(const integer &v) -> integer & = default;
        auto operator =(integer &&v) -> integer & = default;

        auto operator +(const integer &v) const -> integer;
        auto operator -(const integer &v) const -> integer;
        auto operator >>(unsigned int count) const -> integer;
        auto operator <<(unsigned int count) const -> integer;
        auto operator *(const integer &v) const -> integer;
        auto divide_modulus(const integer &v) const -> std::pair<integer, integer>;
        auto compare(const integer &v) const -> integer_comparison;
        auto to_long_long() const -> long long;
        friend auto operator <<(std::ostream &outstr, const integer &v) -> std::ostream &;

        auto is_negative() const -> bool {
            return negative_;
        }

        auto abs() const -> natural {
            return magnitude_;
        }

        auto operator +=(const integer &v) {
            *this = *this + v;
        }

        auto operator -() const -> integer {
            integer res = *this;
            res.negative_ ^= true;
            return res;
        }

        auto operator -=(const integer &v) {
            *this = *this - v;
        }

        auto operator >>=(unsigned int count) {
            *this = *this >> count;
        }

        auto operator <<=(unsigned int count) {
            *this = *this << count;
        }

        auto operator *=(const integer &v) {
            *this = *this * v;
        }

        auto operator /(const integer &v) const -> integer {
            std::pair<integer, integer> dm = divide_modulus(v);
            return dm.first;
        }

        auto operator /=(const integer &v) {
            *this = *this / v;
        }

        auto operator %(const integer &v) const -> integer {
            std::pair<integer, integer> dm = divide_modulus(v);
            return dm.second;
        }

        auto operator %=(const integer &v) {
            *this = *this % v;
        }

        auto operator ==(const integer &v) const -> bool {
            return compare(v) == integer_comparison::eq;
        }

        auto operator !=(const integer &v) const -> bool {
            return compare(v) != integer_comparison::eq;
        }

        auto operator >(const integer &v) const -> bool {
            return compare(v) == integer_comparison::gt;
        }

        auto operator >=(const integer &v) const -> bool {
            return compare(v) != integer_comparison::lt;
        }

        auto operator <(const integer &v) const -> bool {
            return compare(v) == integer_comparison::lt;
        }

        auto operator <=(const integer &v) const -> bool {
            return compare(v) != integer_comparison::gt;
        }

    private:
        bool negative_;                     // Is this big integer negative?
        natural magnitude_;                 // The magnitude of the integer
    };

    inline auto is_negative(integer v) -> bool {
        return v.is_negative();
    }

    inline auto abs(integer v) -> natural {
        return v.abs();
    }

    inline auto to_long_long(const integer &v) -> long long {
        return v.to_long_long();
    }
}

#endif /* __C8_INTEGER_H */

