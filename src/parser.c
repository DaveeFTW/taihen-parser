/*
 * parser.c - parser algorithm for taihen configuration files
 *
 * Copyright (C) 2016 David "Davee" Morgan
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <taihen/parser.h>
#include <taihen/lexer.h>

#ifndef NO_STRING
#include <string.h>
#endif // NO_STRING

static const char *TOKEN_ALL_SECTION = "ALL";
static const char *TOKEN_KERNEL_SECTION = "KERNEL";

#ifdef NO_STRING
#include <stddef.h>

static size_t strlen(const char *s)
{
    size_t idx = 0;

    while (s[idx])
    {
        ++idx;
    }

    return idx;
}

static int strcmp(const char * s1, const char * s2)
{
    while ((*s1) && (*s1 == *s2))
    {
        ++s1;
        ++s2;
    }
    return (*s1 - *s2);
}

#endif // NO_STRING

static inline int is_continuation_byte(char b)
{
    return ((b & 0xC0) == 0x80);
}

static inline int check_continuation_bytes(const char *start, const char *end, int len)
{
    if ((end - start) < len)
    {
        return 0;
    }

    for (int i = 0; i < len; ++i)
    {
        if (!is_continuation_byte(start[i]))
        {
            return 0;
        }
    }

    return 1;
}

static int check_utf8_sequence(const char *str, const char *end, unsigned char mask, unsigned char lead, int cont_len)
{
    if ((*str & mask) == lead)
    {
        if (check_continuation_bytes(str+1, end, cont_len))
        {
            return -1;
        }

        return 1;
    }

    return 0;
}

static int check_utf8(const char *str)
{
    struct
    {
        unsigned char mask;
        unsigned char lead;
        unsigned char cont_len;
    } utf8_lut[4] =
    {
        { 0x80, 0x00, 0 }, // U+0000 -> U+007F, 0xxxxxx
        { 0xE0, 0xC0, 1 }, // U+0080 -> U+07FF, 110xxxxx
        { 0xF0, 0xE0, 2 }, // U+0800 -> U+FFFF, 1110xxxx
        { 0xF8, 0xF0, 3 }, // U+10000 -> U+10FFFF, 11110xxx
    };

    const char *end = str + strlen(str);

    while (str < end)
    {
        int i = 0;

        for (i = 0; i < 4; ++i)
        {
            int res = check_utf8_sequence(str, end, utf8_lut[i].mask, utf8_lut[i].lead, utf8_lut[i].cont_len);

            // check if valid sequence but incorrect contiunation
            if (res < 0)
            {
                return 0;
            }

            // check if valid sequence
            if (res > 0)
            {
                str += utf8_lut[i].cont_len+1;
                break;
            }
        }

        // check if we had no valid sequences
        if (i == 4)
        {
            return 0;
        }
    }

    return 1;
}

/*!
    \brief Check whether a configuration is valid syntax.

    taihen_config_validate is used to check whether a provided configuration is valid syntax.
    This is useful when used before taihen_config_parse to provide error checking before stream based
    parsing.

    \param input A UTF-8 encoded null-terminated string containing the configuration to check.
    \return non-zero on valid configuration, else zero on invalid.
    \sa taihen_config_parse
 */
int taihen_config_validate(const char *input)
{
    taihen_config_lexer ctx;
    taihen_config_init_lexer(&ctx, input);

    int have_section = 0;
    int lex_result = 0;

    while ((lex_result = taihen_config_lex(&ctx)) > 0)
    {
        switch (ctx.token)
        {
        case CONFIG_SECTION_NAME_TOKEN:
            // ensure we actually have a string
            if (strlen(ctx.line_pos) == 0)
            {
                return 0;
            }

            // validate it is UTF-8
            if (!check_utf8(ctx.line_pos))
            {
                return 0;
            }

            have_section = 1;
            break;

        case CONFIG_PATH_TOKEN:
            if (!have_section)
            {
                // paths must belong to a section
                return 0;
            }

            // ensure we actually have a string
            if (strlen(ctx.line_pos) == 0)
            {
                return 0;
            }

            // validate it is UTF-8
            if (!check_utf8(ctx.line_pos))
            {
                return 0;
            }

            break;

        // ignore these, nothing to check
        case CONFIG_SECTION_HALT_TOKEN:
        case CONFIG_COMMENT_TOKEN:
        case CONFIG_SECTION_TOKEN:
        case CONFIG_END_TOKEN:
            break;

        // unexpected tokens, invalid document
        default:
            return 0;
        }
    }

    return (lex_result == 0);
}

/*!
    \brief taihen_config_parse parses a configuration for contextualised paths.

    taihen_config_parse is used to obtain an ordered stream of the paths appropriate for the section provided.
    Special sections such as ALL and KERNEL will be taken into consideration when generating the stream.

    taihen_config_parse provides no error checking or handling. Use taihen_config_validate before parsing the
    document to avoid errors in parsing.

   \param input     A UTF-8 encoded null-terminated string containing the configuration to parse.
   \param section   A UTF-8 encoded null-terminated string containing the section to base context from.
   \param handler   A taihen_config_handler to receive the stream of paths.
   \param param     A user provided value that is passed to the provided taihen_config_handler.
   \sa taihen_config_validate
 */
void taihen_config_parse(const char *input, const char *section, taihen_config_handler handler, void *param)
{
    taihen_config_lexer ctx;
    taihen_config_init_lexer(&ctx, input);

    int halt_flag = 0;
    int record_entries = 0;

    while (taihen_config_lex(&ctx) > 0)
    {
        switch (ctx.token)
        {
        case CONFIG_SECTION_HALT_TOKEN:
            halt_flag = 1;
            break;

        case CONFIG_SECTION_NAME_TOKEN:
            if (strcmp(ctx.line_pos, TOKEN_ALL_SECTION) == 0 && strcmp(section, TOKEN_KERNEL_SECTION) != 0)
            {
                record_entries = 1;
            }
            else if (strcmp(section, ctx.line_pos) == 0)
            {
                record_entries = 1;
            }
            else
            {
                record_entries = 0;
            }

            break;

        case CONFIG_SECTION_TOKEN:
            if (record_entries && halt_flag)
            {
                return;
            }

            halt_flag = 0;
            break;

        case CONFIG_PATH_TOKEN:
            if (record_entries)
            {
                handler(ctx.line_pos, param);
            }

            break;

        default:
            break;
        }
    }
}
