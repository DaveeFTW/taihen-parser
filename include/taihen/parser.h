#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* taihen_config_handler)(const char *module, void *param);

int taihen_config_validate(const char *input);
void taihen_config_parse(const char *input, const char *section, taihen_config_handler handler, void *param);

#ifdef __cplusplus
}
#endif
#endif // PARSER_H
