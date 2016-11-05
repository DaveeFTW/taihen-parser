/*
 * lexer.c - tokenisation algorithm for taihen configuration files
 *
 * Copyright (C) 2016 David "Davee" Morgan
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <taihen/lexer.h>

#ifndef NO_STRING
#include <string.h>
#endif // NO_STRING
#ifndef NO_CTYPE
#include <ctype.h>
#endif // NO_CTYPE

static const char TOKEN_EMPTY = '\0';
static const char TOKEN_COMMENT_START = '#';
static const char TOKEN_SECTION_START = '*';
static const char TOKEN_HALT = '!';

#ifdef NO_CTYPE
static int isspace(int c)
{
    // we use "C"  locale
    return      (c == ' ')
            ||  (c == '\t')
            ||  (c == '\n')
            ||  (c == '\v')
            ||  (c == '\f')
            ||  (c == '\r');
}
#endif // NO_CTYPE

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

static void *memset(void *s, int c, size_t len)
{
    unsigned char *p = (unsigned char *)s;

    while (len--)
    {
        *p++ = (unsigned char)c;
    }

    return s;
}

static void *memcpy(void *s1, const void * s2, size_t len)
{
    char *dest = (char *)s1;
    const char *src = (const char *)s2;

    while (len--)
    {
        *dest++ = *src++;
    }

    return s1;
}
#endif // NO_STRING

static char *skip_whitespace(char *input)
{
    while (isspace((unsigned char)*input))
    {
        ++input;
    }

    return input;
}

static void trim_whitespace(char *input)
{
    char *end = input + strlen(input)-1;

    while (end > input)
    {
        if (!isspace((unsigned char)*end))
        {
            break;
        }

        *end = '\0';
        end--;
    }
}

static const char *get_newline(const char *input)
{
    while (*input)
    {
        if (*input == '\r' || *input == '\n')
        {
            break;
        }

        ++input;
    }

    return input;
}

static int lex_line(taihen_config_lexer *ctx)
{
    if (ctx->input >= ctx->end)
    {
        ctx->token = CONFIG_END_TOKEN;
        return 0;
    }

    const char *line_end = get_newline(ctx->input);
    size_t len = line_end - ctx->input;


    // check our line can fit in our buffer
    if (len >= CONFIG_MAX_LINE_LENGTH)
    {
        return -1;
    }

    // copy line to our buffer so we can modify it
    memcpy(ctx->line, ctx->input, len);
    ctx->line[len] = '\0';
    ctx->line_pos = ctx->line;
    ctx->input = line_end+1;

    // remove leading whitespace
    ctx->line_pos = skip_whitespace(ctx->line_pos);

    // check for empty line or comment
    if (*ctx->line_pos == TOKEN_EMPTY || *ctx->line_pos == TOKEN_COMMENT_START)
    {
        ctx->token = CONFIG_COMMENT_TOKEN;
        return 1;
    }

    // remove any trailing whitespace
    trim_whitespace(ctx->line_pos);

    // check if our line is empty now
    if (*ctx->line_pos == TOKEN_EMPTY)
    {
        ctx->token = CONFIG_COMMENT_TOKEN;
        return 1;
    }

    // check for section start
    if (*ctx->line_pos == TOKEN_SECTION_START)
    {
        ctx->token = CONFIG_SECTION_TOKEN;
    }
    else
    {
        // should be a path
        ctx->token = CONFIG_PATH_TOKEN;
    }

    return 1;
}

static int lex_section_halt(taihen_config_lexer *ctx)
{
    // skip more whitespace
    ctx->line_pos = skip_whitespace(ctx->line_pos+1);

    // check for halt token
    if (*ctx->line_pos == TOKEN_HALT)
    {
        ctx->token = CONFIG_SECTION_HALT_TOKEN;
    }
    else
    {
        // should be a name
        ctx->token = CONFIG_SECTION_NAME_TOKEN;
    }

    return 1;
}

static int lex_section_name(taihen_config_lexer *ctx)
{
    // skip more whitespace
    ctx->line_pos = skip_whitespace(ctx->line_pos+1);

    // should be a name
    ctx->token = CONFIG_SECTION_NAME_TOKEN;
    return 1;
}

/*!
    \brief Initialise or reset lexer context.

    taihen_config_init_lexer will init/reset the provided taihen_config_lexer and assign the
    provided input to the context.

    \param ctx      A non-null pointer to a context to initialise or reset.
    \param input    A non-null UTF-8 encoded null-terminated string to tokenise.
    \return zero on success, < 0 on error.
 */
int taihen_config_init_lexer(taihen_config_lexer *ctx, const char *input)
{
    if (ctx == NULL || input == NULL)
    {
        return -1;
    }

    // reset everything to default and reset input/end pointer
    memset(ctx, 0, sizeof(taihen_config_lexer));
    ctx->token = CONFIG_START_TOKEN;
    ctx->input = input;
    ctx->end = input + strlen(input);
    return 0;
}

/*!
    \brief Retrieve the next lexer token.

    taihen_config_lex will accept an initialised context and provide the next token
    in the stream. This tokenisation does no checking on formatting and as such does not
    confirm that the document provided is well-formed.

    \param ctx  A non-null point to an initialised context.
    \return 0 if there are no further tokens, > 0 if there are further tokens else < 0 on error.
    \sa taihen_config_init_lexer
 */
int taihen_config_lex(taihen_config_lexer *ctx)
{
    if (ctx == NULL)
    {
        return -1;
    }

    switch (ctx->token)
    {
    case CONFIG_START_TOKEN:
    case CONFIG_COMMENT_TOKEN:
    case CONFIG_PATH_TOKEN:
    case CONFIG_SECTION_NAME_TOKEN:
        return lex_line(ctx);

    case CONFIG_SECTION_TOKEN:
        return lex_section_halt(ctx);

    case CONFIG_SECTION_HALT_TOKEN:
        return lex_section_name(ctx);

    case CONFIG_END_TOKEN:
    default:
        return -1;
    }
}
