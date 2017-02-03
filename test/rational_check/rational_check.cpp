/*
 * rational_check.c
 */
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <unistd.h>

#include <rational.h>

/*
 * Result class.
 */
class result {
public:
    result(const std::string &name) {
        name_ = name;
    }

    auto get_name() -> const std::string & {
        return name_;
    }

    auto get_expected() -> const std::string & {
        return expected_;
    }

    auto get_pass() -> bool {
        return pass_;
    }

    auto set_pass(bool p) -> void {
        pass_ = p;
    }

    auto check_pass(const std::string &expected) -> bool {
        expected_ = expected;
        pass_ = (expected == s_.str());
        return pass_;
    }

    auto get_stream() -> std::stringstream & {
        return s_;
    }

    auto start_clock() -> void {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    auto stop_clock() -> void {
        end_time_ = std::chrono::high_resolution_clock::now();
    }

    auto get_elapsed() -> std::chrono::high_resolution_clock::duration {
        return end_time_ - start_time_;
    }

private:
    std::string name_;                  // Test name
    std::string expected_;              // Expected result
    bool pass_;                         // Did this test pass?
    std::chrono::high_resolution_clock::time_point start_time_;
                                        // Start time
    std::chrono::high_resolution_clock::time_point end_time_;
                                        // End time
    std::stringstream s_;               // Output string stream
};

/*
 * Test Constructors.
 */
auto test_construct_0() -> result {
    result r("cons 0");

    r.start_clock();
    c8::rational v(0, 1);
    r.stop_clock();

    r.get_stream() << v;
    r.check_pass("0/1");
    return r;
}

/*
 * Test Constructors.
 */
auto test_construct_1() -> result {
    result r("cons 1");

    r.start_clock();
    c8::rational v(1024, 384);
    r.stop_clock();

    r.get_stream() << v;
    r.check_pass("8/3");
    return r;
}

/*
 * Test Constructors.
 */
auto test_construct_2() -> result {
    result r("cons 2");

    r.start_clock();
    c8::rational v(-1313, 39);
    r.stop_clock();

    r.get_stream() << v;
    r.check_pass("-101/3");
    return r;
}

/*
 * Test Constructors.
 */
auto test_construct_3() -> result {
    result r("cons 3");

    r.start_clock();
    c8::rational v("-99999187951591759827592578257927598275980/25");
    r.stop_clock();

    r.get_stream() << v;
    r.check_pass("-19999837590318351965518515651585519655196/5");
    return r;
}

/*
 * Test Constructors.
 */
auto test_construct_4() -> result {
    result r("cons 4");

    r.start_clock();
    c8::rational v("2/5154875894574578457805710875418754097512875120572105234652346059");
    r.stop_clock();

    r.get_stream() << v;
    r.check_pass("2/5154875894574578457805710875418754097512875120572105234652346059");
    return r;
}

/*
 * Test Constructors.
 */
auto test_construct_5() -> result {
    result r("cons 5");

    r.start_clock();
    c8::rational v("0x2/0x2000000000000000000000000000000000000000000000000000000000000000");
    r.stop_clock();

    r.get_stream() << std::hex << std::showbase << v;
    r.check_pass("0x1/0x1000000000000000000000000000000000000000000000000000000000000000");
    return r;
}

/*
 * Attempt to construct with an invalid octal string.
 */
auto test_construct_6() -> result {
    result r("cons 6");

    r.start_clock();
    try {
        c8::rational v("2323/01185415157637671751");
        r.stop_clock();

        r.get_stream() << "failed to throw exception";
        r.set_pass(false);
    } catch (const c8::invalid_argument &e) {
        r.stop_clock();

        r.get_stream() << "exception thrown: " + std::string(e.what());
        r.set_pass(true);
    } catch (...) {
        r.stop_clock();

        r.get_stream() << "unexpected exception thrown";
        r.set_pass(false);
    }

    return r;
}

/*
 * Test Constructors.
 */
auto test_construct_7() -> result {
    result r("cons 7");

    r.start_clock();
    c8::rational v(1.125);
    r.stop_clock();

    r.get_stream() << v;
    r.check_pass("9/8");
    return r;
}

/*
 * Test Constructors.
 */
auto test_construct_8() -> result {
    result r("cons 8");

    r.start_clock();
    c8::rational v(-1.0/1048576);
    r.stop_clock();

    r.get_stream() << v;
    r.check_pass("-1/1048576");
    return r;
}

/*
 * Attempt to construct with an invalid rational using a double precision infinity.
 */
auto test_construct_9() -> result {
    result r("cons 9");

    r.start_clock();
    try {
        c8::rational v(std::numeric_limits<double>::quiet_NaN());
        r.stop_clock();

        r.get_stream() << "failed to throw exception";
        r.set_pass(false);
    } catch (const c8::not_a_number &e) {
        r.stop_clock();

        r.get_stream() << "exception thrown: " + std::string(e.what());
        r.set_pass(true);
    } catch (...) {
        r.stop_clock();

        r.get_stream() << "unexpected exception thrown";
        r.set_pass(false);
    }

    return r;
}

/*
 * Construct a rational using an imprecise double.
 */
auto test_construct_10() -> result {
    result r("cons 10");

    r.start_clock();
    c8::rational v(0.1);
    r.stop_clock();

    r.get_stream() << std::hex << v;
    r.check_pass("ccccccccccccd/80000000000000");
    return r;
}

/*
 * Add two positive values.
 */
auto test_add_0() -> result {
    result r("add 0");
    c8::rational a0("31/3");
    c8::rational a1("42/3");

    r.start_clock();
    auto a2 = a0 + a1;
    r.stop_clock();

    r.get_stream() << a2;
    r.check_pass("73/3");
    return r;
}

/*
 * Add a positive and a negative value.
 */
auto test_add_1() -> result {
    result r("add 1");
    c8::rational a0("42/13");
    c8::rational a1("-1/2");

    r.start_clock();
    auto a2 = a0 + a1;
    r.stop_clock();

    r.get_stream() << a2;
    r.check_pass("71/26");
    return r;
}

/*
 * Add a positive and a negative value that gives a negative result
 */
auto test_add_2() -> result {
    result r("add 2");
    c8::rational a0("12345678/287923");
    c8::rational a1("-34738957485741895748957485743809574812345678/287923");

    r.start_clock();
    auto a2 = a0 + a1;
    r.stop_clock();

    r.get_stream() << a2;
    r.check_pass("-34738957485741895748957485743809574800000000/287923");
    return r;
}

/*
 * Add two very large negative values.
 */
auto test_add_3() -> result {
    result r("add 3");
    c8::rational a0("-10000000000000000000000000000000000000000000000000000000000000000008789/3");
    c8::rational a1("-88888880000000000000000000000000000000000000000000000000000000999992000/31");

    r.start_clock();
    auto a2 = a0 + a1;
    r.stop_clock();

    r.get_stream() << a2;
    r.check_pass("-192222213333333333333333333333333333333333333333333333333333334333416153/31");
    return r;
}

/*
 * Subtract a 1 digit value from another 1 digit value.
 */
auto test_subtract_0() -> result {
    result r("sub 0");
    c8::rational s0(52, 3);
    c8::rational s1(2, 4);

    r.start_clock();
    auto s2 = s0 - s1;
    r.stop_clock();

    r.get_stream() << s2;
    r.check_pass("101/6");
    return r;
}

/*
 * Subtract a large negative value from another large negative value.
 */
auto test_subtract_1() -> result {
    result r("sub 1");
    c8::rational s0("-5872489572457574027439274027348275342809754320711018574807407090990940275827586671651690897/3");
    c8::rational s1("-842758978027689671615847509157087514875097509475029454785478748571507457514754190754/3");

    r.start_clock();
    auto s2 = s0 - s1;
    r.stop_clock();

    r.get_stream() << s2;
    r.check_pass("-5872488729698595999749602411500766185722239445613509099777952305512191704320129156897500143/3");
    return r;
}

/*
 * Subtract a large negative value from a large value.
 */
auto test_subtract_2() -> result {
    result r("sub 2");
    c8::rational s0("10000000000000000000000000000000000000000000000000000000000000000000000/707");
    c8::rational s1("-10000000000000000000000000000000000000000000000000000000000000000000000/7");

    r.start_clock();
    auto s2 = s0 - s1;
    r.stop_clock();

    r.get_stream() << s2;
    r.check_pass("1020000000000000000000000000000000000000000000000000000000000000000000000/707");
    return r;
}

/*
 * Subtract a large value from a smaller one.
 */
auto test_subtract_3() -> result {
    result r("sub 3");
    c8::rational s0(2, 31459);
    c8::rational s1(52, 31459);

    r.start_clock();
    auto s2 = s0 - s1;
    r.stop_clock();

    r.get_stream() << s2;
    r.check_pass("-50/31459");
    return r;
}

/*
 * Compare two positive values.
 */
auto test_compare_0a() -> result {
    result r("comp 0a");
    c8::rational co0(2, 303);
    c8::rational co1(1, 303);

    r.start_clock();
    auto co2 = (co0 == co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare two positive values.
 */
auto test_compare_0b() -> result {
    result r("comp 0b");
    c8::rational co0(2, 303);
    c8::rational co1(1, 303);

    r.start_clock();
    auto co2 = (co0 != co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare two positive values.
 */
auto test_compare_0c() -> result {
    result r("comp 0c");
    c8::rational co0(2, 303);
    c8::rational co1(1, 303);

    r.start_clock();
    auto co2 = (co0 > co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare two positive values.
 */
auto test_compare_0d() -> result {
    result r("comp 0d");
    c8::rational co0(2, 303);
    c8::rational co1(1, 303);

    r.start_clock();
    auto co2 = (co0 >= co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare two positive values.
 */
auto test_compare_0e() -> result {
    result r("comp 0e");
    c8::rational co0(2, 303);
    c8::rational co1(1, 303);

    r.start_clock();
    auto co2 = (co0 < co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare two positive values.
 */
auto test_compare_0f() -> result {
    result r("comp 0f");
    c8::rational co0(2, 303);
    c8::rational co1(1, 303);

    r.start_clock();
    auto co2 = (co0 <= co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare a negative value with a positive one.
 */
auto test_compare_1a() -> result {
    result r("comp 1a");
    c8::rational co0(-0x987654321LL, 2787539);
    c8::rational co1(1, 1);

    r.start_clock();
    auto co2 = (co0 == co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare a negative value with a positive one.
 */
auto test_compare_1b() -> result {
    result r("comp 1b");
    c8::rational co0(-0x987654321LL, 2787539);
    c8::rational co1(1, 1);

    r.start_clock();
    auto co2 = (co0 != co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare a negative value with a positive one.
 */
auto test_compare_1c() -> result {
    result r("comp 1c");
    c8::rational co0(-0x987654321LL, 2787539);
    c8::rational co1(1, 1);

    r.start_clock();
    auto co2 = (co0 > co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare a negative value with a positive one.
 */
auto test_compare_1d() -> result {
    result r("comp 1d");
    c8::rational co0(-0x987654321LL, 2787539);
    c8::rational co1(1, 1);

    r.start_clock();
    auto co2 = (co0 >= co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare a negative value with a positive one.
 */
auto test_compare_1e() -> result {
    result r("comp 1e");
    c8::rational co0(-0x987654321LL, 2787539);
    c8::rational co1(1, 1);

    r.start_clock();
    auto co2 = (co0 < co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare a negative value with a positive one.
 */
auto test_compare_1f() -> result {
    result r("comp 1f");
    c8::rational co0(-0x987654321LL, 2787539);
    c8::rational co1(1, 1);

    r.start_clock();
    auto co2 = (co0 <= co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare a positive value with a negative one.
 */
auto test_compare_2a() -> result {
    result r("comp 2a");
    c8::rational co0(1, 32787158);
    c8::rational co1(-0x987654321LL, 3);

    r.start_clock();
    auto co2 = (co0 == co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare a positive value with a negative one.
 */
auto test_compare_2b() -> result {
    result r("comp 2b");
    c8::rational co0(1, 32787158);
    c8::rational co1(-0x987654321LL, 3);

    r.start_clock();
    auto co2 = (co0 != co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare a positive value with a negative one.
 */
auto test_compare_2c() -> result {
    result r("comp 2c");
    c8::rational co0(1, 32787158);
    c8::rational co1(-0x987654321LL, 3);

    r.start_clock();
    auto co2 = (co0 > co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare a positive value with a negative one.
 */
auto test_compare_2d() -> result {
    result r("comp 2d");
    c8::rational co0(1, 32787158);
    c8::rational co1(-0x987654321LL, 3);

    r.start_clock();
    auto co2 = (co0 >= co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare a positive value with a negative one.
 */
auto test_compare_2e() -> result {
    result r("comp 2e");
    c8::rational co0(1, 32787158);
    c8::rational co1(-0x987654321LL, 3);

    r.start_clock();
    auto co2 = (co0 < co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare a positive value with a negative one.
 */
auto test_compare_2f() -> result {
    result r("comp 2f");
    c8::rational co0(1, 32787158);
    c8::rational co1(-0x987654321LL, 3);

    r.start_clock();
    auto co2 = (co0 <= co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare two negative values.
 */
auto test_compare_3a() -> result {
    result r("comp 3a");
    c8::rational co0(-0x2f987654321LL);
    c8::rational co1(-0x2f987654321LL);

    r.start_clock();
    auto co2 = (co0 == co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare two negative values.
 */
auto test_compare_3b() -> result {
    result r("comp 3b");
    c8::rational co0(-0x2f987654321LL);
    c8::rational co1(-0x2f987654321LL);

    r.start_clock();
    auto co2 = (co0 != co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare two negative values.
 */
auto test_compare_3c() -> result {
    result r("comp 3c");
    c8::rational co0(-0x2f987654321LL);
    c8::rational co1(-0x2f987654321LL);

    r.start_clock();
    auto co2 = (co0 > co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare two negative values.
 */
auto test_compare_3d() -> result {
    result r("comp 3d");
    c8::rational co0(-0x2f987654321LL);
    c8::rational co1(-0x2f987654321LL);

    r.start_clock();
    auto co2 = (co0 >= co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Compare two negative values.
 */
auto test_compare_3e() -> result {
    result r("comp 3e");
    c8::rational co0(-0x2f987654321LL);
    c8::rational co1(-0x2f987654321LL);

    r.start_clock();
    auto co2 = (co0 < co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("0");
    return r;
}

/*
 * Compare two negative values.
 */
auto test_compare_3f() -> result {
    result r("comp 3f");
    c8::rational co0(-0x2f987654321LL);
    c8::rational co1(-0x2f987654321LL);

    r.start_clock();
    auto co2 = (co0 <= co1);
    r.stop_clock();

    r.get_stream() << co2;
    r.check_pass("1");
    return r;
}

/*
 * Test multiplication.
 */
auto test_multiply_0() -> result {
    result r("mul 0");
    c8::rational mu0(1, 50);
    c8::rational mu1(1, 25);

    r.start_clock();
    auto mu2 = mu0 * mu1;
    r.stop_clock();

    r.get_stream() << mu2;
    r.check_pass("1/1250");
    return r;
}

/*
 * Test multiplication.
 */
auto test_multiply_1() -> result {
    result r("mul 1");
    c8::rational mu0(1000000000000000000LL, 7);
    c8::rational mu1("-9999999999999999999/999");

    r.start_clock();
    auto mu2 = mu0 * mu1;
    r.stop_clock();

    r.get_stream() << mu2;
    r.check_pass("-1111111111111111111000000000000000000/777");
    return r;
}

/*
 * Test multiplication.
 */
auto test_multiply_2() -> result {
    result r("mul 2");
    c8::rational mu0(-0x3000000000000000LL, 1);
    c8::rational mu1(0x4000000000000000LL, 3);

    r.start_clock();
    auto mu2 = mu0 * mu1;
    r.stop_clock();

    r.get_stream() << std::hex << mu2;
    r.check_pass("-4000000000000000000000000000000/1");
    return r;
}

/*
 * Test multiplication.
 */
auto test_multiply_3() -> result {
    result r("mul 3");
    c8::rational mu0("-12345678901234567890123456789012345678901234567890123456789012345678901234567890/13");
    c8::rational mu1("-1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890/13");

    r.start_clock();
    auto mu2 = mu0 * mu1;
    r.stop_clock();

    r.get_stream() << mu2;
    r.check_pass("15241578753238836750495351562566681945008382873376009755225118122311263526910001371743100137174310012193273126047859425087639153757049236500533455762536198787501905199875019052100/169");
    return r;
}

/*
 * Test division.
 */
auto test_divide_0() -> result {
    result r("div 0");
    c8::rational d0(1000000000000000000LL,7);
    c8::rational d1(99999999999999999LL,7);

    r.start_clock();
    auto d2 = d0 / d1;
    r.stop_clock();

    r.get_stream() << d2;
    r.check_pass("1000000000000000000/99999999999999999");
    return r;
}

auto test_divide_1() -> result {
    result r("div 1");
    c8::rational d0("7829238792751875818917817519758789749174743847389742871867617465710657162/39");
    c8::rational d1(-99999999999999999LL, 39);

    r.start_clock();
    auto d2 = d0 / d1;
    r.stop_clock();

    r.get_stream() << d2;
    r.check_pass("-7829238792751875818917817519758789749174743847389742871867617465710657162/99999999999999999");
    return r;
}

auto test_divide_2() -> result {
    result r("div 2");
    c8::rational d0("-0x100000000000000000000000000000000000000000000000000000000000000000000000/2323978529");
    c8::rational d1("0x10000000000000001000000000000000100000000/0x17");

    r.start_clock();
    auto d2 = d0 / d1;
    r.stop_clock();

    r.get_stream() << std::hex << d2;
    r.check_pass("-17000000000000000000000000000000000000000000000000000000000000000/8a851921000000008a851921000000008a851921");
    return r;
}

auto test_divide_3() -> result {
    result r("div 3");

    /*
     * Divide by zero.  This will throw an exception!
     */
    c8::rational d0(2000,7);
    c8::rational d1(0,1);
    r.start_clock();
    try {
        auto d2 = d0 / d1;
        r.stop_clock();

        r.get_stream() << "failed to throw exception";
        r.set_pass(false);
    } catch (const c8::divide_by_zero &e) {
        r.stop_clock();

        r.get_stream() << "exception thrown: " + std::string(e.what());
        r.set_pass(true);
    } catch (...) {
        r.stop_clock();

        r.get_stream() << "unexpected exception thrown";
        r.set_pass(false);
    }

    return r;
}

auto test_divide_4() -> result {
    result r("div 4");
    c8::rational d0(-1000000000000000000LL,23923758279);
    c8::rational d1(-99999999999999999LL,28279753);

    r.start_clock();
    auto d2 = d0 / d1;
    r.stop_clock();

    r.get_stream() << d2;
    r.check_pass("28279753000000000000000000/2392375827899999976076241721");
    return r;
}

/*
 * Test to_double functionality.
 */
auto test_to_double_0() -> result {
    result r("todouble 0");
    c8::rational v(0, 1);

    r.start_clock();
    double d = to_double(v);
    r.stop_clock();

    r.get_stream() << d;
    r.check_pass("0");
    return r;
}

/*
 * Test to_double functionality.
 */
auto test_to_double_1() -> result {
    result r("todouble 1");
    c8::rational v(-3000, 59);

    r.start_clock();
    double d = to_double(v);
    r.stop_clock();

    r.get_stream() << d;
    r.check_pass("-50.8475");
    return r;
}

/*
 * Test to_double functionality.
 */
auto test_to_double_2() -> result {
    result r("todouble 2");
    c8::rational v("47895748574857485728747548237543205782573485472759047548275024574207/389275892758257298");

    r.start_clock();
    double d = to_double(v);
    r.stop_clock();

    r.get_stream() << d;
    r.check_pass("1.23038e+50");
    return r;
}

/*
 * Test to_double functionality.
 */
auto test_to_double_3() -> result {
    result r("todouble 3");
    c8::rational v(0.1);

    r.start_clock();
    double d = to_double(v);
    r.stop_clock();

    r.get_stream() << d;
    r.check_pass("0.1");
    return r;
}

/*
 * Test to_double functionality.
 */
auto test_to_double_4() -> result {
    result r("todouble 4");

    c8::integer i = 1;
    c8::rational v(i << 2048, c8::natural(1));
    r.start_clock();
    try {
        double d = to_double(v);
        r.stop_clock();

        r.get_stream() << d << " (failed to throw exception)";
        r.set_pass(false);
    } catch (const c8::overflow_error &e) {
        r.stop_clock();

        r.get_stream() << "exception thrown: " + std::string(e.what());
        r.set_pass(true);
    } catch (...) {
        r.stop_clock();

        r.get_stream() << "unexpected exception thrown";
        r.set_pass(false);
    }

    return r;
}

/*
 * Test to_parts functionality.
 */
auto test_to_parts_0() -> result {
    result r("toparts 0");
    c8::rational v(0, 1);

    r.start_clock();
    auto x = to_parts(v);
    r.stop_clock();

    r.get_stream() << x.first << ',' << x.second;
    r.check_pass("0,1");
    return r;
}

/*
 * Test to_parts functionality.
 */
auto test_to_parts_1() -> result {
    result r("toparts 1");
    c8::rational v(-3000, 58);

    r.start_clock();
    auto x = to_parts(v);
    r.stop_clock();

    r.get_stream() << x.first << ',' << x.second;
    r.check_pass("-1500,29");
    return r;
}

/*
 * Test printing.
 */
auto test_print_0() -> result {
    result r("prn 0");
    c8::rational v("-0xfedcfedc0123456789/65689");

    r.start_clock();
    r.get_stream() << v;
    r.stop_clock();

    r.check_pass("-4701397401952099592073/65689");
    return r;
}

/*
 * Test printing.
 */
auto test_print_1() -> result {
    result r("prn 1");
    c8::rational v("-0xfedcfedc0123456789/65689");

    r.start_clock();
    r.get_stream() << std::hex << v;
    r.stop_clock();

    r.check_pass("-fedcfedc0123456789/10099");
    return r;
}

/*
 * Test printing.
 */
auto test_print_2() -> result {
    result r("prn 2");
    c8::rational v("-0xfedcfedc0123456789/65689");

    r.start_clock();
    r.get_stream() << std::uppercase << std::hex << v;
    r.stop_clock();

    r.check_pass("-FEDCFEDC0123456789/10099");
    return r;
}

/*
 * Test printing.
 */
auto test_print_3() -> result {
    result r("prn 3");
    c8::rational v("-0xfedcfedc0123456789/65689");

    r.start_clock();
    r.get_stream() << std::oct << v;
    r.stop_clock();

    r.check_pass("-775563766700044321263611/200231");
    return r;
}

/*
 * Test printing.
 */
auto test_print_4() -> result {
    result r("prn 4");
    c8::rational v("-0xfedcfedc0123456789/65689");

    r.start_clock();
    r.get_stream() << std::showbase << v;
    r.stop_clock();

    r.check_pass("-4701397401952099592073/65689");
    return r;
}

/*
 * Test printing.
 */
auto test_print_5() -> result {
    result r("prn 5");
    c8::rational v("-0xfedcfedc0123456789/65689");

    r.start_clock();
    r.get_stream() << std::showbase << std::hex << v;
    r.stop_clock();

    r.check_pass("-0xfedcfedc0123456789/0x10099");
    return r;
}

/*
 * Test printing.
 */
auto test_print_6() -> result {
    result r("prn 6");
    c8::rational v("-0xfedcfedc0123456789/65689");

    r.start_clock();
    r.get_stream() << std::showbase << std::uppercase << std::hex << v;
    r.stop_clock();

    r.check_pass("-0XFEDCFEDC0123456789/0X10099");
    return r;
}

/*
 * Test printing.
 */
auto test_print_7() -> result {
    result r("prn 7");
    c8::rational v("-0xfedcfedc0123456789/65689");

    r.start_clock();
    r.get_stream() << std::showbase << std::oct << v;
    r.stop_clock();

    r.check_pass("-0775563766700044321263611/0200231");
    return r;
}

/*
 * Report the usage for this test program.
 */
static auto usage(const char *name) -> void {
    std::cerr << "usage: " << name << " [OPTIONS]\n\n";
    std::cerr << "Options\n";
    std::cerr << "  -v  Verbose reporting (optional)\n\n";
}

typedef result (*test)();

/*
 * List of tests to run.
 */
test tests[] = {
    test_construct_0,
    test_construct_1,
    test_construct_2,
    test_construct_3,
    test_construct_4,
    test_construct_5,
    test_construct_6,
    test_construct_7,
    test_construct_8,
    test_construct_9,
    test_construct_10,
    test_add_0,
    test_add_1,
    test_add_2,
    test_add_3,
    test_subtract_0,
    test_subtract_1,
    test_subtract_2,
    test_subtract_3,
    test_compare_0a,
    test_compare_0b,
    test_compare_0c,
    test_compare_0d,
    test_compare_0e,
    test_compare_0f,
    test_compare_1a,
    test_compare_1b,
    test_compare_1c,
    test_compare_1d,
    test_compare_1e,
    test_compare_1f,
    test_compare_2a,
    test_compare_2b,
    test_compare_2c,
    test_compare_2d,
    test_compare_2e,
    test_compare_2f,
    test_compare_3a,
    test_compare_3b,
    test_compare_3c,
    test_compare_3d,
    test_compare_3e,
    test_compare_3f,
    test_multiply_0,
    test_multiply_1,
    test_multiply_2,
    test_multiply_3,
    test_divide_0,
    test_divide_1,
    test_divide_2,
    test_divide_3,
    test_divide_4,
    test_to_double_0,
    test_to_double_1,
    test_to_double_2,
    test_to_double_3,
    test_to_double_4,
    test_to_parts_0,
    test_to_parts_1,
    test_print_0,
    test_print_1,
    test_print_2,
    test_print_3,
    test_print_4,
    test_print_5,
    test_print_6,
    test_print_7,
    nullptr
};

/*
 * Entry point.
 */
auto main(int argc, char **argv) -> int {
    bool verbose = false;

    /*
     * Parse the command line options.
     */
    int ch;
    while ((ch = getopt(argc, argv, "v?")) != -1) {
        switch (ch) {
        case 'v':
            verbose = true;
            break;

        case '?':
            usage(argv[0]);
            exit(-1);
        }
    }

    /*
     * Run the tests.
     */
    bool res = true;

    test *p = tests;
    while (*p) {
        result r = (*p)();
        bool rp = r.get_pass();
        if (verbose) {
            std::cout << std::setw(10) << std::left << r.get_name() << " | ";
            std::cout << std::setw(10) << std::right << r.get_elapsed().count() << " | " << (rp ? "pass" : "FAIL");
            std::cout << " | " << r.get_stream().str();
            if (!rp) {
                std::cout << " (" << r.get_expected() << ')';
            }

            std::cout << '\n';
        }

        res &= rp;
        p++;
    }

    if (!res) {
        std::cout << "TESTS FAILED!\n";
        exit(-1);
    }

    std::cout << "All tests passed\n";
    return 0;
}

