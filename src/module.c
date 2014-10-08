#include<dlfcn.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "server.h"

char* module_dir;
struct server_module* module_open(const char* module_name){
    char* module_path;
    void* handle;
    void (* module_generate) (int);
    struct server_module* module;
    module_path = (char*) xmalloc(strlen(module_dir) + strlen(module_name) + 2);
    sprintf(module_path,"%s/%s",module_path,module_name);
    handle = dlopen(module_path,RTLD_NOW);
    free(module_path);
    if(handle == NULL)
        return NULL;
    module_generate = (void (*) (int)) dlsym(handle,"module_generate");
    if(module_generate == NULL){
        dlclose(handle);
        return NULL;
    }
    module = (struct server_module*) xmalloc (sizeof(struct server_module));
    module->handle = handle;
    module->name = xstrdup(module_name);
    module->generate_function = module_generate;
    return module;

}

void module_close(struct server_module* module){
    dlclose(module->handle);
    free((char*) module->name);
    free(module);

}
