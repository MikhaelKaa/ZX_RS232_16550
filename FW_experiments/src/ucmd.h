#ifndef _UCMD_H_
#define _UCMD_H_

// #include <limits.h>

#define UCMD_CMD_NOT_FOUND INT_MIN

typedef int (*command_cb)(int, char **);

typedef struct command {
    const char *cmd;    /**< the command string to match against */
    const char *help;   /**< the help text associated with cmd */
    command_cb fn;      /**< the function to call when cmd is matched */
} command_t;

// Init.
void ucmd_default_init(void);

// call in loop.
void ucmd_default_proc(void);


int ucmd_parse(command_t[], int argc, const char **argv);

int print_help_cb(int argc, char *argv[]);

void ucmd_default_print(const char * str);

void default_sigint(void);


void ucmd_set_sigint(void (*sigintf)(void));
void default_sigint(void);
int ucmd_execute(int argc, char **argv);

// https://github.com/thefekete/uCmd

#endif /* _UCMD_H_ */
