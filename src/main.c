#include<assert.h>
#include<getopt.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>

#include "server.h"

static const struct option long_options[] = {
    {"address",1,NULL,'a'},
    {"help",0,NULL,'h'},
    {"module-dir",1,NULL,'m'},
    {"port",1,NULL,'P'},
    {"verbose",0,NULL,'v'},
};
static const char* const short_options = "a:hm:p:v";
static const char* const usage_template = 
    "Usage:%s [options]\n"
    "-a, --address ADDR    Bind to local address\n"
    "-h, --help  print this help infomation\n"
    "-m, --module-dir DIR\n"
    "-p,--port PORT\n"
    "-v,--verbose  print verbose message\n";

static void print_usage(int is_error){
    fprintf(is_error?stderr:stdout,usage_template,program_name);
    exit(is_error?1:0);
}

int main (int argc,char* const argv[]){
    struct in_addr local_address;
    uint16_t port;
    int next_options;
    program_name = argv[0];
    local_address.s_addr = IMADDR_ANY;
    port = 0;
    verbose = 0;
    module_dir = get_self_executable_directory();
    assert(module_dir != NULL);
    do{
        next_option = getopt_long(argc,argv,short_options,long_options,NULL);
        switch(next_option){
            case 'a':
                {
                    struct hostent* local_host_name;
                    local_host_name = gethostbyname(optarg);
                    if(local_host_name == NULL || local_host_name->h_length == 0)
                        error(optarg,"invaild host name");
                    else
                        local_address.s_addr = *((int*) (local_host_name->h_addr_list[0]));

                }
                break;
            case 'h':
                print_usage(0);
            case 'm':
                {
                    struct stat dir_info;
                    if(access (optarg,F_OK) != 0)
                        error(optarg,"module directory does not exist.");
                    if(access(optarg,R_OK | X_OK) != 0) 
                        error(optarg,"module directory is not accessible.");
                    if(stat(optarg,&dir_info) != 0 || !S_ISDIR(dir_info.st_mode))
                        error(optarg,"not a directory.");
                    module_dir = strdup(optarg);

                }
                break;
            case 'p':
                {
                    long value;
                    char* end;
                    value = strtol (optarg, &end, 10);
                    if (*end != '\0')
                        print_usage(1);
                    port = (uint16_t) htons (value);
                }
                break;
            case 'v':
                verbose = 1;
                break;
            case '?':
                print_usage(1);
            case -1:
                break;
            default:abort();


        }


    }while(next_option != -1);
    
    if(optind != argc)
        print_usage(1);
    if(verbose)
        print("module will be loaded from %s\n",module_dir);
    server_run(local_address,port);

    return 0;

    


}
