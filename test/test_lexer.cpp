#include <taihen/lexer.h>

#define BOOST_TEST_MODULE lexer
#include <boost/test/unit_test.hpp>

#include <random>
#include <iomanip>

BOOST_AUTO_TEST_CASE(init_lexer)
{
    const char *input = "";
    taihen_config_lexer ctx;

    // test NULL parameter handling
    BOOST_REQUIRE_LT(taihen_config_init_lexer(NULL, NULL), 0);
    BOOST_REQUIRE_LT(taihen_config_init_lexer(&ctx, NULL), 0);
    BOOST_REQUIRE_LT(taihen_config_init_lexer(NULL, input), 0);

    // test correct input
    BOOST_REQUIRE_GE(taihen_config_init_lexer(&ctx, input), 0);
}

BOOST_AUTO_TEST_CASE(empty_lex)
{
    const char *input = "";
    taihen_config_lexer ctx;

    BOOST_WARN_GE(taihen_config_init_lexer(&ctx, input), 0);

    // we should expect immediate end of stream
    BOOST_REQUIRE_EQUAL(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_END_TOKEN);
}

BOOST_AUTO_TEST_CASE(reset_lexer)
{
    const char *input = "";
    taihen_config_lexer ctx;

    BOOST_WARN_GE(taihen_config_init_lexer(&ctx, input), 0);

    // we should expect immediate end of stream
    BOOST_WARN_EQUAL(taihen_config_lex(&ctx), 0);
    BOOST_WARN_EQUAL(ctx.token, CONFIG_END_TOKEN);

    // reset the lexer
    BOOST_REQUIRE_GE(taihen_config_init_lexer(&ctx, input), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_START_TOKEN);
}

BOOST_AUTO_TEST_CASE(simple_section_lex)
{
    const char *input = "*MY SECTION";
    taihen_config_lexer ctx;

    BOOST_WARN_GE(taihen_config_init_lexer(&ctx, input), 0);

    // we should expect section token
    BOOST_REQUIRE_GT(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_SECTION_TOKEN);

    // then we expect name
    BOOST_REQUIRE_GT(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_SECTION_NAME_TOKEN);

    // check name is still "MY SECTION"
    BOOST_TEST(ctx.line_pos == "MY SECTION");

    // then we expect end of stream
    BOOST_REQUIRE_EQUAL(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_END_TOKEN);
}

BOOST_AUTO_TEST_CASE(complex_section_lex)
{
    const char *input = "*!MY SECTION";
    taihen_config_lexer ctx;

    BOOST_WARN_GE(taihen_config_init_lexer(&ctx, input), 0);

    // we should expect section token
    BOOST_REQUIRE_GT(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_SECTION_TOKEN);

    // we should expect section halt token
    BOOST_REQUIRE_GT(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_SECTION_HALT_TOKEN);

    // then we expect name
    BOOST_REQUIRE_GT(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_SECTION_NAME_TOKEN);

    // check name is still "MY SECTION"
    BOOST_TEST(ctx.line_pos == "MY SECTION");

    // then we expect end of stream
    BOOST_REQUIRE_EQUAL(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_END_TOKEN);
}


BOOST_AUTO_TEST_CASE(whitespace_lex)
{
    const char *input = "\t\t    \t\t";
    taihen_config_lexer ctx;

    BOOST_WARN_GE(taihen_config_init_lexer(&ctx, input), 0);

    // we should expect comment token
    BOOST_REQUIRE_GT(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_COMMENT_TOKEN);

    BOOST_REQUIRE_EQUAL(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_END_TOKEN);
}

BOOST_AUTO_TEST_CASE(comment_lex)
{
    const char *input = "#this is a comment";
    taihen_config_lexer ctx;

    BOOST_WARN_GE(taihen_config_init_lexer(&ctx, input), 0);

    // we should expect comment token
    BOOST_REQUIRE_GT(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_COMMENT_TOKEN);

    BOOST_REQUIRE_EQUAL(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_END_TOKEN);
}

BOOST_AUTO_TEST_CASE(path_lex)
{
    const char *input = "this:/is/a/path";
    taihen_config_lexer ctx;

    BOOST_WARN_GE(taihen_config_init_lexer(&ctx, input), 0);

    // we should expect path token, this isnt valid config syntax
    // but its not lexer job to ensure its correct order
    // it just tokenises the input
    BOOST_REQUIRE_GT(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_PATH_TOKEN);

    BOOST_REQUIRE_EQUAL(taihen_config_lex(&ctx), 0);
    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_END_TOKEN);
}

BOOST_AUTO_TEST_CASE(random_lex)
{
    std::random_device seed;
    std::mt19937_64 mt;
    std::vector<unsigned char> line(255);
    taihen_config_lexer ctx;

    // seed mt from random device
    mt.seed(seed());

    for (auto i = 0; i < 100000; ++i)
    {
        std::generate(line.begin(), std::prev(line.end()), mt);

        line[254] = '\0';

        BOOST_WARN_GE(taihen_config_init_lexer(&ctx, (char *)(line.data())), 0);

        while(1)
        {
            int res = taihen_config_lex(&ctx);

            if (res < 0)
            {
                std::stringstream ss;

                ss << "on generated data: " << std::hex << std::setfill('0');

                std::for_each(line.begin(), line.end(), [&ss](auto& v)
                {
                    ss << std::setw(2) << static_cast<unsigned>(v);
                });

                ss << std::endl;


                BOOST_TEST_REQUIRE(res >= 0, ss.str());
            }

            if (res == 0)
            {
                break;
            }
        }
    }

    BOOST_REQUIRE_EQUAL(ctx.token, CONFIG_END_TOKEN);
}

BOOST_AUTO_TEST_CASE(long_line_lex)
{
    char line[CONFIG_MAX_LINE_LENGTH+1];
    taihen_config_lexer ctx;

    std::memset(line, 'a', sizeof(line));
    line[CONFIG_MAX_LINE_LENGTH] = '\0';

    BOOST_REQUIRE_GE(taihen_config_init_lexer(&ctx, line), 0);
    BOOST_REQUIRE_LT(taihen_config_lex(&ctx), 0);
}
