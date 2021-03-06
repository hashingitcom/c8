/*
 * rational.cpp
 */
#include "c8.h"

namespace c8 {
    /*
     * Construct a rational from an IEEE754 double precision floating point value.
     */
    rational::rational(double v) {
        /*
         * Convert our double into a bit representation.
         *
         * Note: As currently written the code only works for a little-endian CPU.
         */
        uint64_t *vbits_ptr = reinterpret_cast<uint64_t *>(&v);
        uint64_t vbits = *vbits_ptr;

        /*
         * With a double precisions IEEE754 floating point value we have:
         *
         * bits 0-51:   Significand
         * bits 52-62:  Exponent
         * bit 63:      Sign
         */
        bool neg = (vbits & 0x8000000000000000ULL) ? true : false;
        vbits &= 0x7fffffffffffffffULL;
        int64_t exp = static_cast<int64_t>(vbits >> 52) - 1023;
        int64_t sig = static_cast<int64_t>(vbits & 0x000fffffffffffffULL);

        /*
         * Is this an infinity or a NaN?
         */
        if (exp == 1024) {
            throw not_a_number();
        }

        /*
         * Do we have a normalized value (as opposed to a denormalized one)?  Most
         * floating point values are normalized.
         */
        if (exp != -1022) {
            /*
             * For normal numbers we have an implied 52nd bit set.  Set that
             * explicitly now!  When we do that though we're also shifting our
             * exponent by 52 places too.
             */
            sig |= 0x0010000000000000ULL;
            exp -= 52;
        }

        if (neg) {
            sig = -sig;
        }

        /*
         * Create our numerator and denominator.  If our exponent is negative then
         * we scale our denominator up, but if the exponent is positive then we
         * scan our numerator instead.
         */
        integer i = integer(sig);

        if (exp < 0) {
            numerator_ = i;
            denominator_ = integer(1) << static_cast<unsigned int>(-exp);
        } else {
            numerator_ = i << static_cast<unsigned int>(exp);
            denominator_ = integer(1);
        }

        normalize();
    }

    /*
     * Construct a rational using a string.
     *
     * The string can have an optional '-' sign to indicate that it's negative,
     * and then the usual C++-like hex, octal, * or decimal representations.
     * The numerator and denominator are separated by a '/', and there are no
     * optional spaces.
     */
    rational::rational(const std::string &v) {
        /*
         * Do we have a '/' character separating a numerator and denominator?
         */
        std::size_t pos = v.find(std::string("/"));
        if (pos == std::string::npos) {
            numerator_ = integer(v);
        } else {
            numerator_ = integer(v.substr(0, pos));
            denominator_ = integer(v.substr(pos + 1));
        }

        normalize();
    }

    /*
     * Return true if this rational is equal to another one, false if it's not.
     */
    auto rational::operator ==(const rational &v) const -> bool {
        /*
         * As we always keep our numbers normalized then checking equivalence
         * simply means comparing the numerators and denominators to see if they're
         * different or not.
         */
        return (numerator_ == v.numerator_) && (denominator_ == v.denominator_);
    }

    /*
     * Return true if this rational is not equal to another one, false if it's equal.
     */
    auto rational::operator !=(const rational &v) const -> bool {
        /*
         * As we always keep our numbers normalized then checking non-equivalence
         * simply means comparing the numerators and denominators to see if they're
         * different or not.
         */
        return (numerator_ != v.numerator_) || (denominator_ != v.denominator_);
    }

    /*
     * Return true if this rational is greater than another one, false if it's not.
     */
    auto rational::operator >(const rational &v) const -> bool {
        return (numerator_ * v.denominator_) > (v.numerator_ * denominator_);
    }

    /*
     * Return true if this rational is greater than, or equal to, another one, false if it's not.
     */
    auto rational::operator >=(const rational &v) const -> bool {
        return (numerator_ * v.denominator_) >= (v.numerator_ * denominator_);
    }

    /*
     * Return true if this rational is less than another one, false if it's not.
     */
    auto rational::operator <(const rational &v) const -> bool {
        return (numerator_ * v.denominator_) < (v.numerator_ * denominator_);
    }

    /*
     * Return true if this rational is less than, or equal to, another one, false if it's not.
     */
    auto rational::operator <=(const rational &v) const -> bool {
        return (numerator_ * v.denominator_) <= (v.numerator_ * denominator_);
    }

    /*
     * Add another rational to this one.
     */
    auto rational::operator +(const rational &v) const -> rational {
        rational res;

        res.numerator_ = (numerator_ * v.denominator_) + (denominator_ * v.numerator_);
        res.denominator_ = denominator_ * v.denominator_;

        res.normalize();
        return res;
    }

    /*
     * Add another rational to this one.
     */
    auto rational::operator +=(const rational &v) -> rational & {
        numerator_ *= v.denominator_;
        numerator_ += (denominator_ * v.numerator_);
        denominator_ *= v.denominator_;

        normalize();
        return *this;
    }

    /*
     * Subtract another rational from this one.
     */
    auto rational::operator -(const rational &v) const -> rational {
        rational res;

        res.numerator_ = (numerator_ * v.denominator_) - (denominator_ * v.numerator_);
        res.denominator_ = denominator_ * v.denominator_;

        res.normalize();
        return res;
    }

    /*
     * Subtract another rational from this one.
     */
    auto rational::operator -=(const rational &v) -> rational & {
        numerator_ *= v.denominator_;
        numerator_ -= (denominator_ * v.numerator_);
        denominator_ *= v.denominator_;

        normalize();
        return *this;
    }

    /*
     * Multiply another rational with this one.
     */
    auto rational::operator *(const rational &v) const -> rational {
        rational res;

        res.numerator_ = numerator_ * v.numerator_;
        res.denominator_ = denominator_ * v.denominator_;

        res.normalize();
        return res;
    }

    /*
     * Multiply another rational with this one.
     */
    auto rational::operator *=(const rational &v) -> rational & {
        numerator_ *= v.numerator_;
        denominator_ *= v.denominator_;

        normalize();
        return *this;
    }

    /*
     * Divide this rational by another one.
     */
    auto rational::operator /(const rational &v) const -> rational {
        rational res;

        /*
         * Are we attempting to divide by zero?  If we are then throw an exception.
         */
        if (v.is_zero()) {
            throw c8::divide_by_zero();
        }

        res.numerator_ = numerator_ * v.denominator_;
        res.denominator_ = denominator_ * v.numerator_;

        res.normalize();
        return res;
    }

    /*
     * Divide this rational by another one.
     */
    auto rational::operator /=(const rational &v) -> rational & {
        /*
         * Are we attempting to divide by zero?  If we are then throw an exception.
         */
        if (v.is_zero()) {
            throw c8::divide_by_zero();
        }

        numerator_ *= v.denominator_;
        denominator_ *= v.numerator_;

        normalize();
        return *this;
    }

    /*
     * Normalize the data.
     */
    auto rational::normalize() -> void {
        if (C8_UNLIKELY(is_negative(denominator_))) {
            numerator_.negate();
            denominator_.negate();
        }

        /*
         * Find the GCD of the numerator and denominator.
         */
        integer g = gcd(numerator_, denominator_);
        numerator_ /= g;
        denominator_ /= g;
    }

    /*
     * Convert this rational to a double.
     */
    auto rational::to_double() const -> double {
        /*
         * Is our value zero?  If it is then handle this as a special case.
         */
        natural n = numerator_.magnitude();
        if (n.is_zero()) {
            return 0.0;
        }

        /*
         * We need to get a dividend that is sufficiently large that when it's
         * divided by our denominator that we get 52 bits of result.  We don't
         * want our division operation to be too expensive either, so one of the
         * other things we can do is scale our denominator down.
         */
        natural d = denominator_.magnitude();
        int eshift = 0;
        auto dbits = d.size_bits();
        if (dbits > 52) {
            std::size_t s = dbits - 52;
            eshift -= static_cast<int>(s);
            dbits = 52;
            d >>= static_cast<unsigned int>(s);
        }

        auto nbits = n.size_bits();
        if (nbits > dbits + 53) {
            std::size_t s = nbits - (dbits + 53);
            n >>= static_cast<unsigned int>(s);
            eshift += static_cast<int>(s);
        } else {
            std::size_t s = dbits + 53 - nbits;
            n <<= static_cast<unsigned int>(s);
            eshift -= static_cast<int>(s);
        }

        /*
         * Compute the signifcand.
         */
        natural sig = n / d;

        /*
         * Convert the significand to an uin64_t.  It will either have 53 or 54
         * bits.  If it's 54 bits then shift right by one.  Once we have 53 bits
         * we can strip bit 52.
         */
        uint64_t res = static_cast<uint64_t>(to_unsigned_long_long(sig));
        while (res > 0x0020000000000000ULL) {
            res >>= 1;
            eshift++;
        }

        res &= 0x000fffffffffffffULL;

        eshift += 52;

        /*
         * Will this number fit in a double?  If not then throw an exception.
         */
        if (eshift > 1024) {
            throw overflow_error();
        }

        if (eshift < -1022) {
            throw underflow_error();
        }

        res |= (static_cast<uint64_t>(eshift + 1023) << 52);

        /*
         * Is our numerator negative?
         */
        if (is_negative(numerator_)) {
            res |= 0x8000000000000000ULL;
        }

        /*
         * Convert our result bitmap into a double representation.
         *
         * Note: As currently written the code only works for a little-endian CPU.
         */
        double *dbits_ptr = reinterpret_cast<double *>(&res);
        return *dbits_ptr;
    }

    /*
     * << operator to print a rational.
     */
    auto operator <<(std::ostream &outstr, const rational &v) -> std::ostream & {
        outstr << v.numerator_ << '/' << v.denominator_;

        return outstr;
    }
}

