// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jcr/jcr_validator.hpp"

using namespace jsoncons;
using namespace jsoncons::jcr;

BOOST_AUTO_TEST_SUITE(jcr_test_suite)

BOOST_AUTO_TEST_CASE(test_jcr)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {
        "line-count" : 3426,
        "word-count" : 27886
    }
    )");

    json val1 = json::parse(R"(
    {
        "line-count" : 3426,
        "word-count" : 27886
    }
    )");
    json val2 = json::parse(R"(
    {
        "line-count" : 3426,
        "word-count" : 27887
    }
    )");

    BOOST_CHECK(schema.validate(val1));
    BOOST_CHECK(!schema.validate(val2));
}

BOOST_AUTO_TEST_CASE(test_jcr_integer)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {
        "line-count" : integer,
        "word-count" : integer
    }
    )");

    json val1 = json::parse(R"(
    {
        "line-count" : 3426,
        "word-count" : 27886
    }
    )");

    BOOST_CHECK(schema.validate(val1));
}

BOOST_AUTO_TEST_CASE(test_jcr_integer_range)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {
        "line-count" : 0..,
        "word-count" : 0..
    }
    )");

    json val1 = json::parse(R"(
    {
        "line-count" : 3426,
        "word-count" : 27886
    }
    )");

    BOOST_CHECK(schema.validate(val1));

    jcr_validator schema2 = jcr_validator::parse(R"(
    {
        "line-count" : 3427..,
        "word-count" : 0..
    }
    )");

    BOOST_CHECK(!schema2.validate(val1));
    
}

BOOST_AUTO_TEST_CASE(test_jcr_string)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {
        "file-name"  : string,
        "line-count" : 0..,
        "word-count" : 0..
    }
    )");

    json val1 = json::parse(R"(
    {
        "file-name"  : "rfc7159.txt",
        "line-count" : 3426,
        "word-count" : 27886
    }
    )");

    BOOST_CHECK(schema.validate(val1));
}

BOOST_AUTO_TEST_CASE(test_named_rules)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {
        fn,
        lc,
        wc
    }
    fn "file-name"  : string
    lc "line-count" : 0..
    wc "word-count" : 0..    
    )");

    json val1 = json::parse(R"(
    {
        "file-name"  : "rfc7159.txt",
        "line-count" : 3426,
        "word-count" : 27886
    }
    )");

    BOOST_CHECK(schema.validate(val1));
} 
 
BOOST_AUTO_TEST_CASE(test_named_rules2)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {
        fn,
        lc,
        wc
    }
    fn "file-name"  : "rfc7159.txt"
    lc "line-count" : 3426
    wc "word-count" : 27886
    )");

    json val1 = json::parse(R"(
    {
        "file-name"  : "rfc7159.txt",
        "line-count" : 3426,
        "word-count" : 27886
    }
    )");

    BOOST_CHECK(schema.validate(val1));
}

BOOST_AUTO_TEST_CASE(test_member_range_value_rule)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {
        v1
    }
    v1 "value"  : 0..3
    )");

    json val1 = json::parse(R"(
    {
        "value"  : 1
    }
    )");

    json val2 = json::parse(R"(
    {
        "value"  : -1
    }
    )");

    json val3 = json::parse(R"(
    {
        "value"  : 4
    }
    )");

    BOOST_CHECK(schema.validate(val1));
    BOOST_CHECK(!schema.validate(val2));
    BOOST_CHECK(!schema.validate(val3));
}

BOOST_AUTO_TEST_CASE(test_range_value_rule)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {
        m1
    }
    v1 : 0..3
    m1 "value" : v1
    )");

    json val1 = json::parse(R"(
    {
        "value"  : 1
    }
    )");

    json val2 = json::parse(R"(
    {
        "value"  : -1
    }
    )");

    json val3 = json::parse(R"(
    {
        "value"  : 4
    }
    )");

    BOOST_CHECK(schema.validate(val1));
    BOOST_CHECK(!schema.validate(val2));
    BOOST_CHECK(!schema.validate(val3));
}

BOOST_AUTO_TEST_CASE(test_optional_rule)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {o1}
    v1 : 0..3
    m1 "m1" : v1
    m2 "m2" : v1
    o1 "m0" : { m1, m2 }
    )");

    json val1 = json::parse(R"(
    {
        "m0" : {"m1":1,"m2":2}
    }
    )");

    BOOST_CHECK(schema.validate(val1));
}

BOOST_AUTO_TEST_CASE(test_optional_member_optional_rule)
{
    jcr_validator schema = jcr_validator::parse(R"(
    {o1}
    v1 : 0..3
    m1 "m1" : v1
    m2 "m2" : v1
    o1 "m0" : { m1, ?m2 }
    )");

    json val1 = json::parse(R"(
    {
        "m0" : {"m1":1,"m2":2}
    }
    )");

    BOOST_CHECK(schema.validate(val1));
}

BOOST_AUTO_TEST_CASE(test_array_rule)
{
    jcr_validator schema = jcr_validator::parse(R"(
    [o1]
    v1 : 0..3
    m1 "m1" : v1
    m2 "m2" : v1
    o1 : { m1, ?m2 }
    )");

    json val1 = json::parse(R"(
    [
        {"m1":1,"m2":2}
    ]
    )");

    BOOST_CHECK(schema.validate(val1));
}

BOOST_AUTO_TEST_SUITE_END()



