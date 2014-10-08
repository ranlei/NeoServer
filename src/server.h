#ifndef SERVER_H
#define SERVER_H

#include<netinet/in.h>
#include<sys/types.h>

/*common.c*/
extern const char* program_name;

extern int verbose;

/*like malloc,realloc,strdup,expect aborts the program if allocation fails.*/

extern void* xmalloc(size_t size);

extern void* xrealloc(void* ptr,size_t size);

extern char* xstrdup(const char* s);

extern void system_error(const char* operation);

extern void error(const char* cause,const char* message);

extern char* get_self_executable_directory();

/*modules.c*/

struct server_module{
    void* handle;
    const char* name;
    void (* generate_function) (int);
};

extern char* module_dir;
extern struct server_module* module_open(const char* module_path);
extern void module_close(struct server_module* module);

/*server.c*/

extern void server_run(struct in_addr local_address,uint16_t port);

#endif

