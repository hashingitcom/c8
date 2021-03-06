# c8::integer::operator % #

```cpp
auto operator %(const integer &v) const -> integer;
```

Divides the value of this object (left hand side of the operator) by `v` (right hand side of the operator), resulting in a new `c8::integer` object that represents the remainder.

### Return Value ###

A new `c8::integer` object with a value equal to the remainder of the value of this object divided by `v`.

### Exceptions ###

* `std::bad_alloc` in the event of a memory allocation failure.
* `c8::divide_by_zero` if the divisor, `v` is zero.

## See Also ##

* [class integer](c8_integer)
* [c8 library](c8)

