/*
 * natural.cpp
 */
#include <cctype>

#include "natural.h"
#include "digit_array.h"

namespace c8 {
    const char digits_lower[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    const char digits_upper[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    /*
     * Reserve a number of digits in this natural number.
     */
    inline auto natural::reserve(std::size_t new_digits) -> void {
        /*
         * If our digit array is already large enough (which is very likely) then
         * we don't need to do anything.
         */
        if (C8_LIKELY(digits_size_ >= new_digits)) {
            return;
        }

        /*
         * Allocate a new digit array and update book-keeping info.
         */
        digits_size_ = new_digits;
        delete_digits_on_final_ = true;
        digits_ = new natural_digit[new_digits];
    }

    /*
     * Expand the number of digits in this natural number.
     */
    inline auto natural::expand(std::size_t new_digits) -> void {
        /*
         * If our digit array is already large enough (which is very likely) then
         * we don't need to do anything.
         */
        if (C8_LIKELY(digits_size_ >= new_digits)) {
            return;
        }

        /*
         * Replace the old digit array with the new one.
         */
        auto d = new natural_digit[new_digits];
        copy_digit_array(d, digits_, num_digits_);

        delete_digits();
        digits_size_ = new_digits;
        delete_digits_on_final_ = true;
        digits_ = d;
    }

    /*
     * Copy the contents of a natural number into this one.
     */
    auto natural::copy_digits(const natural &v) -> void {
        digits_size_ = sizeof(small_digits_) / sizeof(natural_digit);
        delete_digits_on_final_ = false;
        digits_ = small_digits_;
        num_digits_ = v.num_digits_;
        if (C8_UNLIKELY(!num_digits_)) {
            return;
        }

        reserve(v.num_digits_);
        copy_digit_array(digits_, v.digits_, num_digits_);
    }

    /*
     * Steal the contents of a natural number into this one.
     */
    auto natural::steal_digits(natural &v) -> void {
        delete_digits_on_final_ = v.delete_digits_on_final_;

        /*
         * Are we currently using the default small buffer, or do we have one allocated?
         */
        if (C8_UNLIKELY(v.delete_digits_on_final_)) {
            v.delete_digits_on_final_ = false;

            /*
             * We're using an allocated buffer so just move it.
             */
            digits_ = v.digits_;
            v.digits_ = v.small_digits_;
            digits_size_ = v.digits_size_;
            v.digits_size_ = sizeof(v.small_digits_) / sizeof(natural_digit);
        } else {
            /*
             * We're using the default buffer so copy the contents.
             */
            copy_digit_array(small_digits_, v.small_digits_, v.num_digits_);
            digits_ = small_digits_;
            digits_size_ = sizeof(small_digits_) / sizeof(natural_digit);
        }

        num_digits_ = v.num_digits_;
        v.num_digits_ = 0;
    }

    /*
     * Construct a natural number from an unsigned long long integer.
     */
    natural::natural(unsigned long long v) {
        num_digits_ = 0;
        digits_size_ = sizeof(small_digits_) / sizeof(natural_digit);
        digits_ = small_digits_;
        delete_digits_on_final_ = false;

        if (!v) {
            return;
        }

        reserve(sizeof(unsigned long long) / sizeof(natural_digit));

        natural_digit *this_digits = digits_;

        std::size_t i = 0;
        do {
            natural_digit m = static_cast<natural_digit>(-1);
            this_digits[i++] = static_cast<natural_digit>(v & m);
            v >>= natural_digit_bits;
        } while (v);

        num_digits_ = i;
    }

    /*
     * Construct a natural number from a string.
     *
     * The string representation can have the usual C++-like hex, octal, or decimal
     * representations.
     */
    natural::natural(const std::string &v) {
        num_digits_ = 0;
        digits_size_ = sizeof(small_digits_) / sizeof(natural_digit);
        digits_ = small_digits_;
        delete_digits_on_final_ = false;

        std::size_t v_sz = v.size();
        if (C8_UNLIKELY(v_sz == 0)) {
            throw invalid_argument("zero size string");
        }

        natural res;

        /*
         * Is our first character a '0'?  If it is then we may have an octal or hex number.
         */
        std::size_t idx = 0;
        natural_digit base = 10;
        if (v[0] == '0') {
            idx = 1;
            base = 8;
            if (v_sz > 1) {
                auto c2 = v[1];
                if ((c2 == 'x') || (c2 == 'X')) {
                    idx = 2;
                    base = 16;
                }
            }
        }

        /*
         * We now know the base we're using and the starting index.
         */
        for (std::size_t i = idx; i < v_sz; i++) {
            char c = v[i];
            if (!isdigit(c)) {
                if (base != 16) {
                    throw invalid_argument("invalid digit");
                }

                if (!isxdigit(c)) {
                    throw invalid_argument("invalid digit");
                }

                c = static_cast<char>(tolower(c));
                c = static_cast<char>(c - ('a' - '0' - 10));
            }

            if (base == 8) {
                if (c >= '8') {
                    throw invalid_argument("invalid digit");
                }
            }

            res *= base;
            res += static_cast<natural_digit>(c - '0');
        }

        *this = std::move(res);
    }

    /*
     * Return the number of bits required to represent this natural number.
     */
    auto natural::count_bits() const noexcept -> unsigned int {
        return count_bits_digit_array(digits_, num_digits_);
    }

    /*
     * Compare this natural number with another one.
     */
    auto natural::compare(const natural &v) const noexcept -> comparison {
        return compare_digit_arrays(digits_, num_digits_, v.digits_, v.num_digits_);
    }

    /*
     * Add a natural digit to this natural number.
     */
    auto natural::operator +(natural_digit v) const -> natural {
        natural res;

        /*
         * Is v zero?  If yes, then our result is just this number.
         */
        if (!v) {
            res = *this;
            return res;
        }

        std::size_t this_num_digits = num_digits_;
        res.reserve(this_num_digits + 1);

        /*
         * Does this number have zero digits?  If yes, then our result is just v.
         */
        if (!this_num_digits) {
            res.digits_[0] = v;
            res.num_digits_ = 1;
            return res;
        }

        /*
         * Does this number have only one digit?  If yes, then just add that digit and v.
         */
        if (this_num_digits == 1) {
            res.num_digits_ = add_digit_digit(res.digits_, digits_[0], v);
            return res;
        }

        /*
         * Add v to the n digits of this number.
         */
        res.num_digits_ = add_digit_array_digit(res.digits_, digits_, this_num_digits, v);
        return res;
    }

    /*
     * Add another natural number to this one.
     */
    auto natural::operator +(const natural &v) const -> natural {
        natural res;

        /*
         * Does v have zero digits?  If yes, then our result is just this number.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            res = *this;
            return res;
        }

        /*
         * Does this number have zero digits?  If yes, then our result is just v.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            res = v;
            return res;
        }

        /*
         * Does v have only one digit?
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];
            res.reserve(this_num_digits + 1);

            /*
             * Does this number have only one digit?  If yes, just add the two digits.
             */
            if (this_num_digits == 1) {
                res.num_digits_ = add_digit_digit(res.digits_, digits_[0], v_digit);
                return res;
            }

            /*
             * Add v's single digit to the n digits of this number.
             */
            res.num_digits_ = add_digit_array_digit(res.digits_, digits_, this_num_digits, v_digit);
            return res;
        }

        /*
         * Does this number have only one digit?  If yes then add that digit to the n digits of v.
         */
        if (this_num_digits == 1) {
            /*
             * Add this number's single digit to the n digits of v.
             */
            res.reserve(v_num_digits + 1);
            res.num_digits_ = add_digit_array_digit(res.digits_, v.digits_, v_num_digits, digits_[0]);
            return res;
        }

        /*
         * Worst case scenario:  We're adding two arrays of digits.
         */
        res.reserve(v_num_digits + this_num_digits);
        res.num_digits_ = add_digit_arrays(res.digits_, digits_, this_num_digits, v.digits_, v_num_digits);
        return res;
    }

    /*
     * Add a natural digit to this number.
     */
    auto natural::operator +=(natural_digit v) -> natural & {
        /*
         * Is v zero?  If yes, then our result is just this number.
         */
        if (!v) {
            return *this;
        }

        std::size_t this_num_digits = num_digits_;
        expand(this_num_digits + 1);

        /*
         * Does this number have zero digits?  If yes, then our result is just v.
         */
        if (!this_num_digits) {
            digits_[0] = v;
            num_digits_ = 1;
            return *this;
        }

        /*
         * Does this number have only one digit?  If yes, then just add that digit and v.
         */
        if (this_num_digits == 1) {
            num_digits_ = add_digit_digit(digits_, digits_[0], v);
            return *this;
        }

        /*
         * Add v to the n digits of this number.
         */
        num_digits_ = add_digit_array_digit(digits_, digits_, this_num_digits, v);
        return *this;
    }

    /*
     * Add another natural number to this one.
     */
    auto natural::operator +=(const natural &v) -> natural & {
        /*
         * Does v have zero digits?  If yes, then our result is just this number.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            return *this;
        }

        /*
         * Does this number have zero digits?  If yes, then our result is just v.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            *this = v;
            return *this;
        }

        /*
         * Does v have only one digit?
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];
            expand(this_num_digits + 1);

            /*
             * Does v only have only one digit?  If yes, just add the two digits.
             */
            if (this_num_digits == 1) {
                num_digits_ = add_digit_digit(digits_, digits_[0], v_digit);
                return *this;
            }

            /*
             * Add v's single digit to the n digits of this number.
             */
            num_digits_ = add_digit_array_digit(digits_, digits_, this_num_digits, v_digit);
            return *this;
        }

        /*
         * Does this number have only one digit?  If yes then add that digit to the n digits of v.
         */
        if (this_num_digits == 1) {
            expand(v_num_digits + 1);
            num_digits_ = add_digit_array_digit(digits_, v.digits_, v_num_digits, digits_[0]);
            return *this;
        }

        /*
         * Worst case scenario:  We're adding two arrays of digits.
         */
        expand(v_num_digits + this_num_digits);
        num_digits_ = add_digit_arrays(digits_, digits_, this_num_digits, v.digits_, v_num_digits);
        return *this;
    }

    /*
     * Subtract a natural digit from this natural number.
     */
    auto natural::operator -(natural_digit v) const -> natural {
        natural res;

        /*
         * Is v zero?  If yes, then our result is just this number.
         */
        if (!v) {
            res = *this;
            return res;
        }

        /*
         * Does this number have zero digits?  If yes, then we have an exception.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            throw not_a_number();
        }

        res.reserve(this_num_digits);

        /*
         * Does this number have only one digit?  If yes, then subtract v from just that digit.
         */
        if (this_num_digits == 1) {
            if (digits_[0] < v) {
                throw not_a_number();
            }

            res.num_digits_ = subtract_digit_digit(res.digits_, digits_[0], v);
            return res;
        }

        /*
         * Subtract v from the n digits of this number.
         */
        res.num_digits_ = subtract_digit_array_digit(res.digits_, digits_, this_num_digits, v);
        return res;
    }

    /*
     * Subtract another natural number from this one.
     */
    auto natural::operator -(const natural &v) const -> natural {
        natural res;

        /*
         * Does v have zero digits?  If yes, then our result is just this number.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            res = *this;
            return res;
        }

        /*
         * Does this number have zero digits?  If yes, then we have an exception.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            throw not_a_number();
        }

        res.reserve(this_num_digits);

        /*
         * Does v have only one digit?  If yes, then we can use faster approaches.
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];

            /*
             * Does this number have only one digit?  If yes, then subtract v from just that digit.
             */
            if (this_num_digits == 1) {
                if (digits_[0] < v_digit) {
                    throw not_a_number();
                }

                res.num_digits_ = subtract_digit_digit(res.digits_, digits_[0], v_digit);
                return res;
            }

            /*
             * Subtract v from the n digits of this number.
             */
            res.num_digits_ = subtract_digit_array_digit(res.digits_, digits_, this_num_digits, v_digit);
            return res;
        }

        /*
         * We should not have a negative result!
         */
        if (compare_digit_arrays(digits_, this_num_digits, v.digits_, v_num_digits) == comparison::lt) {
            throw not_a_number();
        }

        res.num_digits_ = subtract_digit_arrays(res.digits_, digits_, this_num_digits, v.digits_, v_num_digits);
        return res;
    }

    /*
     * Subtract a natural digit from this natural number.
     */
    auto natural::operator -=(natural_digit v) -> natural & {
        /*
         * Is v zero?  If yes, then our result is just this number.
         */
        if (!v) {
            return *this;
        }

        /*
         * Does this number have zero digits?  If yes, then we have an exception.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            throw not_a_number();
        }

        /*
         * Does this number have only one digit?  If yes, then subtract v from just that digit.
         */
        if (this_num_digits == 1) {
            if (digits_[0] < v) {
                throw not_a_number();
            }

            num_digits_ = subtract_digit_digit(digits_, digits_[0], v);
            return *this;
        }

        /*
         * Subtract v from the n digits of this number.
         */
        num_digits_ = subtract_digit_array_digit(digits_, digits_, this_num_digits, v);
        return *this;
    }

    /*
     * Subtract another natural number from this one.
     */
    auto natural::operator -=(const natural &v) -> natural & {
        /*
         * Does v have zero digits?  If yes, then our result is just this number.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            return *this;
        }

        /*
         * Does this number have zero digits?  If yes, then we have an exception.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            throw not_a_number();
        }

        /*
         * Does v have only one digit?  If yes, then we can use faster approaches.
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];

            /*
             * Does this number have only one digit?  If yes, then subtract v from just that digit.
             */
            if (this_num_digits == 1) {
                if (digits_[0] < v_digit) {
                    throw not_a_number();
                }

                num_digits_ = subtract_digit_digit(digits_, digits_[0], v_digit);
                return *this;
            }

            /*
             * Subtract v from the n digits of this number.
             */
            num_digits_ = subtract_digit_array_digit(digits_, digits_, this_num_digits, v_digit);
            return *this;
        }

        /*
         * We should not have a negative result!
         */
        if (compare_digit_arrays(digits_, this_num_digits, v.digits_, v_num_digits) == comparison::lt) {
            throw not_a_number();
        }

        num_digits_ = subtract_digit_arrays(digits_, digits_, this_num_digits, v.digits_, v_num_digits);
        return *this;
    }

    /*
     * Left shift this natural number by a bit count.
     */
    auto natural::operator <<(unsigned int count) const -> natural {
        natural res;

        /*
         * Does this number have zero digits?  If yes, then our result is zero.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            return res;
        }

        std::size_t trailing_digits = count / natural_digit_bits;
        std::size_t digit_shift = count % natural_digit_bits;

        res.reserve(this_num_digits + trailing_digits + 1);

        /*
         * Does this number have only one digit?  If yes then use a faster approach.
         */
        if (this_num_digits == 1) {
            res.num_digits_ = left_shift_digit(res.digits_, digits_[0], trailing_digits, digit_shift);
            return res;
        }

        /*
         * Worst case:  Shift our digit array.
         */
        res.num_digits_ = left_shift_digit_array(res.digits_, digits_, this_num_digits, trailing_digits, digit_shift);
        return res;
    }

    /*
     * Left shift this natural number by a bit count.
     */
    auto natural::operator <<=(unsigned int count) -> natural & {
        /*
         * Does this number have zero digits?  If yes, then our result is zero.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            return *this;
        }

        std::size_t trailing_digits = count / natural_digit_bits;
        std::size_t digit_shift = count % natural_digit_bits;

        expand(this_num_digits + trailing_digits + 1);

        /*
         * Does this number have only one digit?  If yes then use a faster approach.
         */
        if (this_num_digits == 1) {
            num_digits_ = left_shift_digit(digits_, digits_[0], trailing_digits, digit_shift);
            return *this;
        }

        /*
         * Worst case:  Shift our digit array.
         */
        num_digits_ = left_shift_digit_array(digits_, digits_, this_num_digits, trailing_digits, digit_shift);
        return *this;
    }

    /*
     * Right shift this natural number by a bit count.
     */
    auto natural::operator >>(unsigned int count) const -> natural {
        natural res;

        std::size_t this_num_digits = num_digits_;
        std::size_t trailing_digits = count / natural_digit_bits;
        std::size_t digit_shift = count % natural_digit_bits;

        /*
         * Does this number have fewer digits than we want to shift by?  If yes, then our result is zero.
         */
        if (this_num_digits <= trailing_digits) {
            return res;
        }

        res.reserve(this_num_digits - trailing_digits);

        /*
         * Does this number have only one digit?  If yes, then use a faster approach.
         */
        if (this_num_digits == 1) {
            res.num_digits_ = right_shift_digit(res.digits_, digits_[0], digit_shift);
            return res;
        }

        res.num_digits_ = right_shift_digit_array(res.digits_, digits_, this_num_digits, trailing_digits, digit_shift);
        return res;
    }

    /*
     * Right shift this natural number by a bit count.
     */
    auto natural::operator >>=(unsigned int count) -> natural & {
        std::size_t this_num_digits = num_digits_;
        std::size_t trailing_digits = count / natural_digit_bits;
        std::size_t digit_shift = count % natural_digit_bits;

        /*
         * Does this number have fewer digits than we want to shift by?  If yes, then our result is zero.
         */
        if (this_num_digits <= trailing_digits) {
            num_digits_ = 0;
            return *this;
        }

        /*
         * Does this number have only one digit?  If yes, then use a faster approach.
         */
        if (this_num_digits == 1) {
            num_digits_ = right_shift_digit(digits_, digits_[0], digit_shift);
            return *this;
        }

        num_digits_ = right_shift_digit_array(digits_, digits_, this_num_digits, trailing_digits, digit_shift);
        return *this;
    }

    /*
     * Multiply this natural number with a single digit.
     */
    auto natural::operator *(natural_digit v) const -> natural {
        natural res;

        /*
         * Is v zero?  If yes, then our result is zero too.
         */
        if (!v) {
            return res;
        }

        /*
         * Does this number have zero digits?  If yes, then our result is zero.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            return res;
        }

        res.reserve(this_num_digits + 1);

        /*
         * Does this number have only one digit?  If yes, then just multiply that digit and v.
         */
        if (this_num_digits == 1) {
            res.num_digits_ = multiply_digit_digit(res.digits_, digits_[0], v);
            return res;
        }

        /*
         * Multiply the n digits of this number by v.
         */
        res.num_digits_ = multiply_digit_array_digit(res.digits_, digits_, this_num_digits, v);
        return res;
    }

    /*
     * Multiply this natural number with another one.
     */
    auto natural::operator *(const natural &v) const -> natural {
        natural res;

        /*
         * Does this number have zero digits?  If yes, then our result is zero.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            return res;
        }

        /*
         * Does this number have zero digits?  If yes, then our result is zero.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            return res;
        }

        std::size_t res_num_digits = this_num_digits + v_num_digits;
        res.reserve(res_num_digits);

        /*
         * Does v have only one digit?  If yes, then we can use faster approaches.
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];

            /*
             * Does this number have only one digit?  If yes, then just multiply that digit and v.
             */
            if (this_num_digits == 1) {
                res.num_digits_ = multiply_digit_digit(res.digits_, digits_[0], v_digit);
                return res;
            }

            /*
             * Multiply the n digits of this number by v.
             */
            res.num_digits_ = multiply_digit_array_digit(res.digits_, digits_, this_num_digits, v_digit);
            return res;
        }

        /*
         * Does this number have only one digit?  If yes then multiply that digit and the n digits of v.
         */
        if (this_num_digits == 1) {
            res.num_digits_ = multiply_digit_array_digit(res.digits_, v.digits_, v_num_digits, digits_[0]);
            return res;
        }

        /*
         * Worst case scenario:  We're multiplying two arrays of digits.
         */
        res.num_digits_ = multiply_digit_arrays(res.digits_, digits_, this_num_digits, v.digits_, v_num_digits);
        return res;
    }

    /*
     * Multiply this natural number with a single digit.
     */
    auto natural::operator *=(natural_digit v) -> natural & {
        /*
         * Is v zero?  If yes, then our result is zero too.
         */
        if (!v) {
            num_digits_ = 0;
            return *this;
        }

        /*
         * Does this number have zero digits?  If yes, then our result is zero.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            return *this;
        }

        expand(this_num_digits + 1);

        /*
         * Does this number have only one digit?  If yes, then just multiply that digit and v.
         */
        if (this_num_digits == 1) {
            num_digits_ = multiply_digit_digit(digits_, digits_[0], v);
            return *this;
        }

        /*
         * Multiply the n digits of this number by v.
         */
        num_digits_ = multiply_digit_array_digit(digits_, digits_, this_num_digits, v);
        return *this;
    }

    /*
     * Multiply this natural number with another one.
     */
    auto natural::operator *=(const natural &v) -> natural & {
        /*
         * Does this number have zero digits?  If yes, then our result is zero.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            return *this;
        }

        /*
         * Does this number have zero digits?  If yes, then our result is zero.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            return *this;
        }

        std::size_t res_num_digits = this_num_digits + v_num_digits;
        expand(res_num_digits);

        /*
         * Does v have only one digit?  If yes, then we can use faster approaches.
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];

            /*
             * Does this number have only one digit?  If yes, then just multiply that digit and v.
             */
            if (this_num_digits == 1) {
                num_digits_ = multiply_digit_digit(digits_, digits_[0], v_digit);
                return *this;
            }

            /*
             * Multiply the n digits of this number by v.
             */
            num_digits_ = multiply_digit_array_digit(digits_, digits_, this_num_digits, v_digit);
            return *this;
        }

        /*
         * Does this number have only one digit?  If yes then multiply that digit and the n digits of v.
         */
        if (this_num_digits == 1) {
            num_digits_ = multiply_digit_array_digit(digits_, v.digits_, v_num_digits, digits_[0]);
            return *this;
        }

        /*
         * Worst case scenario:  We're multiplying two arrays of digits.
         */
        natural_digit this_digits[this_num_digits];
        copy_digit_array(this_digits, digits_, this_num_digits);
        num_digits_ = multiply_digit_arrays(digits_, this_digits, this_num_digits, v.digits_, v_num_digits);
        return *this;
    }

    /*
     * Divide this natural number by a natural digit, returning the quotient and remainder.
     */
    auto natural::divide_modulus(natural_digit v) const -> std::pair<natural, natural_digit> {
        /*
         * Are we attempting to divide by zero?  If we are then throw an exception.
         */
        if (!v) {
            throw divide_by_zero();
        }

        std::pair<natural, natural_digit> p;

        /*
         * Is the result zero?  If yes then we're done.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            p.second = 0;
            return p;
        }

        p.first.reserve(this_num_digits);

        /*
         * Does this number have only one digit?  If yes, then divide that digit by v.
         */
        if (this_num_digits == 1) {
            p.first.num_digits_ = divide_modulus_digit_digit(p.first.digits_, p.second, digits_[0], v);
            return p;
        }

        p.first.num_digits_ = divide_modulus_digit_array_digit(p.first.digits_, p.second, digits_, this_num_digits, v);
        return p;
    }

    /*
     * Divide this natural number by another one, returning the quotient and remainder.
     */
    auto natural::divide_modulus(const natural &v) const -> std::pair<natural, natural> {
        /*
         * Are we attempting to divide by zero?  If we are then throw an exception.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            throw divide_by_zero();
        }

        std::pair<natural, natural> p;

        /*
         * Is the result zero?  If yes then we're done.
         */
        std::size_t this_num_digits = num_digits_;
        if (compare_digit_arrays(digits_, this_num_digits, v.digits_, v_num_digits) == comparison::lt) {
            p.second = *this;
            return p;
        }

        std::size_t quotient_num_digits = this_num_digits - v_num_digits + 1;
        p.first.reserve(quotient_num_digits);

        /*
         * Does, our divisor v, only have one digit?  If yes, then use the fast version
         * of divide_modulus.
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];
            natural_digit mod;

            /*
             * Does this number have only one digit?  If yes, then divide that digit by v.
             */
            if (this_num_digits == 1) {
                p.first.num_digits_ = divide_modulus_digit_digit(p.first.digits_, mod, digits_[0], v_digit);
                return p;
            }

            p.first.num_digits_ = divide_modulus_digit_array_digit(p.first.digits_, mod, digits_, this_num_digits, v_digit);
            p.second = natural(mod);
            return p;
        }

        p.first.num_digits_ = quotient_num_digits;

        std::size_t remainder_num_digits = this_num_digits + 1;
        p.second.reserve(remainder_num_digits);
        p.second.num_digits_ = remainder_num_digits;

        divide_modulus_digit_arrays(p.first.digits_, p.first.num_digits_, p.second.digits_, p.second.num_digits_,
                                    digits_, this_num_digits, v.digits_, v_num_digits);
        return p;
    }

    /*
     * Divide this natural number by a single digit, returning the quotient.
     */
    auto natural::operator /(natural_digit v) const -> natural {
        /*
         * Are we attempting to divide by zero?  If we are then throw an exception.
         */
        if (!v) {
            throw divide_by_zero();
        }

        natural quotient;

        /*
         * Is the result zero?  If yes then we're done.
         */
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            return quotient;
        }

        natural_digit remainder;
        quotient.reserve(this_num_digits);

        /*
         * Does this number have only one digit?  If yes, then divide that digit by v.
         */
        if (this_num_digits == 1) {
            quotient.num_digits_ = divide_modulus_digit_digit(quotient.digits_, remainder, digits_[0], v);
            return quotient;
        }

        quotient.num_digits_ = divide_modulus_digit_array_digit(quotient.digits_, remainder, digits_, this_num_digits, v);
        return quotient;
    }

    /*
     * Divide this natural number by another one, returning the quotient.
     */
    auto natural::operator /(const natural &v) const -> natural {
        /*
         * Are we attempting to divide by zero?  If we are then throw an exception.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            throw divide_by_zero();
        }

        natural quotient;

        /*
         * Is the result zero?  If yes then we're done.
         */
        std::size_t this_num_digits = num_digits_;
        if (compare_digit_arrays(digits_, this_num_digits, v.digits_, v_num_digits) == comparison::lt) {
            return quotient;
        }

        std::size_t quotient_num_digits = this_num_digits - v_num_digits + 1;
        quotient.reserve(quotient_num_digits);

        /*
         * Does, our divisor v, only have one digit?  If yes, then use the fast version
         * of divide_modulus.
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];
            natural_digit mod;

            /*
             * Does this number have only one digit?  If yes, then divide that digit by v.
             */
            if (this_num_digits == 1) {
                quotient.num_digits_ = divide_modulus_digit_digit(quotient.digits_, mod, digits_[0], v_digit);
                return quotient;
            }

            quotient.num_digits_ = divide_modulus_digit_array_digit(quotient.digits_, mod, digits_, this_num_digits, v_digit);
            return quotient;
        }

        quotient.num_digits_ = quotient_num_digits;

        std::size_t remainder_num_digits = this_num_digits + 1;
        natural_digit remainder_digits[remainder_num_digits];

        divide_modulus_digit_arrays(quotient.digits_, quotient.num_digits_, remainder_digits, remainder_num_digits,
                                    digits_, this_num_digits, v.digits_, v_num_digits);
        return quotient;
    }

    /*
     * Divide this natural number by a single digit, returning the quotient.
     */
    auto natural::operator /=(natural_digit v) -> natural & {
        *this = *this / v;
        return *this;
    }

    /*
     * Divide this natural number by another one, returning the quotient.
     */
    auto natural::operator /=(const natural &v) -> natural & {
        *this = *this / v;
        return *this;
    }

    /*
     * Divide this natural number by a single digit, returning the remainder.
     */
    auto natural::operator %(natural_digit v) const -> natural_digit {
        /*
         * Are we attempting to divide by zero?  If we are then throw an exception.
         */
        if (!v) {
            throw divide_by_zero();
        }

        natural_digit remainder = 0;
        std::size_t this_num_digits = num_digits_;
        if (!this_num_digits) {
            return remainder;
        }

        natural_digit res_digits[this_num_digits];

        /*
         * Does this number have only one digit?  If yes, then divide that digit by v.
         */
        if (this_num_digits == 1) {
            divide_modulus_digit_digit(res_digits, remainder, digits_[0], v);
            return remainder;
        }

        divide_modulus_digit_array_digit(res_digits, remainder, digits_, this_num_digits, v);
        return remainder;
    }

    /*
     * Divide this natural number by another one, returning the remainder.
     */
    auto natural::operator %(const natural &v) const -> natural {
        /*
         * Are we attempting to divide by zero?  If we are then throw an exception.
         */
        std::size_t v_num_digits = v.num_digits_;
        if (!v_num_digits) {
            throw divide_by_zero();
        }

        natural remainder;

        /*
         * Is the result zero?  If yes then we're done.
         */
        std::size_t this_num_digits = num_digits_;
        if (compare_digit_arrays(digits_, this_num_digits, v.digits_, v_num_digits) == comparison::lt) {
            remainder = *this;
            return remainder;
        }

        std::size_t quotient_num_digits = this_num_digits - v_num_digits + 1;
        natural_digit quotient_digits[quotient_num_digits];

        /*
         * Does, our divisor v, only have one digit?  If yes, then use the fast version
         * of divide_modulus.
         */
        if (v_num_digits == 1) {
            auto v_digit = v.digits_[0];
            natural_digit mod;

            /*
             * Does this number have only one digit?  If yes, then divide that digit by v.
             */
            if (this_num_digits == 1) {
                divide_modulus_digit_digit(quotient_digits, mod, digits_[0], v_digit);
                remainder = natural(mod);
                return remainder;
            }

            divide_modulus_digit_array_digit(quotient_digits, mod, digits_, this_num_digits, v_digit);
            remainder = natural(mod);
            return remainder;
        }

        std::size_t remainder_num_digits = this_num_digits + 1;
        remainder.reserve(remainder_num_digits);
        remainder.num_digits_ = remainder_num_digits;

        divide_modulus_digit_arrays(quotient_digits, quotient_num_digits, remainder.digits_, remainder.num_digits_,
                                    digits_, this_num_digits, v.digits_, v_num_digits);
        return remainder;
    }

    /*
     * Divide this natural number by a single digit, returning the remainder.
     */
    auto natural::operator %=(natural_digit v) -> natural & {
        *this = *this % v;
        return *this;
    }

    /*
     * Divide this natural number by another one, returning the remainder.
     */
    auto natural::operator %=(const natural &v) -> natural & {
        *this = *this % v;
        return *this;
    }

    /*
     * Find the greatest common divisor of this and another natural number.
     */
    auto natural::gcd(const natural &v) const -> natural {
        natural smaller;

        std::size_t v_num_digits = v.num_digits_;
        if (!v.num_digits_) {
            smaller = *this;
            return smaller;
        }

        std::size_t this_num_digits = num_digits_;
        if (!num_digits_) {
            smaller = v;
            return smaller;
        }

        natural larger;
        if (compare_digit_arrays(digits_, this_num_digits, v.digits_, v_num_digits) == comparison::lt) {
            smaller = *this;
            larger = v;
        } else {
            smaller = v;
            larger = *this;
        }

        while (true) {
            natural mod = larger % smaller;
            if (!mod.num_digits_) {
                break;
            }

            larger.delete_digits();
            larger.steal_digits(smaller);
            smaller.steal_digits(mod);
        }

        return smaller;
    }

    /*
     * Convert this natural number to an unsigned long long.
     */
    auto natural::to_unsigned_long_long() const -> unsigned long long {
        /*
         * Will this number fit in an unsigned long long?  If not then throw an
         * exception.
         */
        if (count_bits() > (8 * sizeof(long long))) {
            throw overflow_error();
        }

        /*
         * Convert the value to our result format.
         */
        unsigned long long res = 0;
        std::size_t sz = num_digits_;
        if (sz > (sizeof(unsigned long long) / sizeof(natural_digit))) {
            sz = sizeof(unsigned long long) / sizeof(natural_digit);
        }

        const natural_digit *this_digits = digits_;
        for (unsigned int i = 0; i < sz; i++) {
            res |= (static_cast<unsigned long long>(this_digits[i]) << (i * natural_digit_bits));
        }

        return res;
    }

    /*
     * << operator to print a natural.
     */
    auto operator <<(std::ostream &outstr, const natural &v) -> std::ostream & {
        std::size_t v_num_digits = v.num_digits_;
        if (v_num_digits == 0) {
            outstr << '0';
            return outstr;
        }

        natural_digit base = 10;
        auto flags = outstr.flags();
        if (flags & std::ios_base::hex) {
            base = 16;
        } else if (flags & std::ios_base::oct) {
            base = 8;
        }

        const char *digits = digits_lower;
        if (flags & std::ios_base::uppercase) {
            digits = digits_upper;
        }

        if (flags & std::ios_base::showbase) {
            if (base != 10) {
                outstr << '0';
                if (base == 16) {
                    outstr << ((flags & std::ios_base::uppercase) ? 'X' : 'x');
                }
            }
        }

        std::vector<char> res;
        auto rem = v;
        do {
            std::pair<natural, natural_digit> qm = rem.divide_modulus(base);
            natural_digit mod = qm.second;
            res.emplace_back(digits[mod]);

            rem = std::move(qm.first);
        } while (!is_zero(rem));

        std::size_t res_sz = res.size();
        for (std::size_t i = 0; i < res_sz; i++) {
            outstr << res[res_sz - 1 - i];
        }

        return outstr;
    }
}

