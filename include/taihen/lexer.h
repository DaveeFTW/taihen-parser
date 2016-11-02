#ifndef LEXER_H
#define LEXER_H

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_MAX_LINE_LENGTH (256)

typedef enum
{
    CONFIG_START_TOKEN,
    CONFIG_END_TOKEN,
    CONFIG_COMMENT_TOKEN,
    CONFIG_SECTION_TOKEN,
    CONFIG_SECTION_HALT_TOKEN,
    CONFIG_SECTION_NAME_TOKEN,
    CONFIG_PATH_TOKEN
} taihen_config_lexer_token;

typedef struct
{
    const char *input;
    const char *end;
    taihen_config_lexer_token token;
    char line[CONFIG_MAX_LINE_LENGTH];
    char *line_pos;
} taihen_config_lexer;

int taihen_config_init_lexer(taihen_config_lexer *ctx, const char *input);
int taihen_config_lex(taihen_config_lexer *ctx);

#ifdef __cplusplus
}
#endif
#endif // LEXER_H
