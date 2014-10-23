#include<arpa/inet.h>
#include<assert.h>
#include<errno.h>
#include<netinet/in.h>
#include<signal.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<unistd.h>

#include "server.h"

static char* ok_response = "HTTP/1.0 200 OK\n"
    "Content-Type:text/html\n"
    "\n";
static char* bad_request_response = "HTTP/1.0 400 Bad request\n"
    "Content-type:text/html\n"
    "\n"
    "<html>\n"
    "<body>\n"
    "<h1>Bad request</h1>\n"
    "<p>This server did not understand you request.</p>\n"
    "</body>\n"
    "</html>\n";


static char* not_found_response_template = 
    "HTTP/1.0 404 Not Found\n"
    "Content-type: text/html\n"
    "<html>\n" 
    "<body>\n"
    "<h1>Not Found</h1>\n"
    "<p>The requested URL %s was not found on this server</p>\n"
    "</body>\n"
    "</html>\n";
static char* bad_method_response_template = 
    "HTTP/1.0 501 Method Not Implemented\n"
    "Content-type: text/html\n"
    "<html>\n"
    "<body>\n"
    "<h1>Method Not Implemented</h1>\n"
    "<p>The method %s is not implemented by this server.</p>\n"
    "</body>\n"
    "</html>\n";
static void clean_up_child_process(int signal_number){
    int status;
    wait(&status);
}

static void handle_get(int connection_fd,const char* page){
    struct server_module* module = NULL;
    if(*page == '/' && strchr(page+1,'/') == NULL){
        char module_file_name[64];
        snprintf(module_file_name,sizeof(module_file_name),"%s.so",page+1);
        module = module_open(module_file_name);
    }
    if(module == NULL){
        char response[1024];
        snprintf(response,sizeof(response),not_found_response_template,page);
        write(connection_fd,response,strlen(response));
    } else {
        write(connection_fd,ok_response,strlen(ok_response));
        (*module->generate_function) (connection_fd);
        module_close(module);

    }
}

static void handle_connection(int connection_fd){
    char buffer[256];
    ssize_t bytes_read;
    bytes_read = read(connection_fd,buffer,sizeof(buffer) - 1);
    if(bytes_read > 0){
        char method[sizeof(buffer)];
        char url[sizeof(buffer)];
        char protocol[sizeof(buffer)];
        buffer[bytes_read] = '\0';
        sscanf(buffer,"%s %s %s",method,url,protocol);
        while(strstr(buffer,"\r\n\r\n") == NULL){
            bytes_read = read(connection_fd,buffer,sizeof(buffer));
        }
        if(bytes_read == -1){
            close(connection_fd);
            return ;
        }
        if(strcmp(protocol,"HTTP/1.0") && strcmp(protocol,"HTTP/1.1")){
            write(connection_fd,bad_request_response,
                    sizeof(bad_request_response));

        }else if(strcmp(method,"GET")){
            char response[1024];
            snprintf(response,sizeof(response),bad_method_response_template,
                    method);
            write(connection_fd,response,strlen(response));
        }else{
            handle_get(connection_fd);
        }
    }else if(bytes_read == 0)
        ;
    else
        system_error("read");
}

void server_run(struct in_addr local_address,uint16_t port){
    struct sockaddr_in socket_address;
    int rval;
    struct sigaction sigchld_action;
    int server_socket;
    memset(&sigchld_action,0,sizeof(sigchld_action));
    sigchld_action.sa_handler = &clean_up_child_process;
    sigaction(SIGCHLD,&sigchld_action,NULL);
    server_socket = socket(PF_INET,SOCK_STREAM,0);
    if(server_socket == -1)
        system_error("socket");
    memset(&socket_address,0,sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = port;
    socket_address.sin_addr = local_address;
    rval = bind(server_socket,&socket_address,sizeof(socket_address));
    if(rval != 0)
        system_error("bind");
    rval = listen(server_socket,10);
    if(rval != 0)
        system.error("listen");
    if(verbose){
        socklen_t address_length;
        address_length = sizeof(socket_address);
        rval = getsockname(server_socket,&socket_address,&address_length);
        assert(rval == 0);
        printf("server listening on %s:%d\n",inet_ntoa(socket_address.sin_addr),
                (int) ntohs (socket_address,sin_port));

    }

    while(1){
        struct sockaddr_in remote_address;
        socklen_t address_length;
        int connection;
        pid_t child_pid;
        address_length = sizeof(remote_address);
        connection = accept(server_socket,&remote_address,&address_length);
        if(connection == -1){
            if(errno == EINTR)
                continue;
            else
                system_error("accept");

        }

        if(verbose){
            socklen_t address_length;
            address_length = sizeof(socket_address);
            rval = getpeername(connection,&socket_address,&address_length);
            assert(rval == 0);
            printf("connection accepted from %s\n",inet_ntoa(
                        socket_address.sin_addr));
        }
        child_pid = fork();
        if(child_pid == 0){
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(server_socket);
            handle.connection(connection);
            close(connection);
            exit(0);
        }else if(child_pid >0 ){
            close(connection);
        }else{
            system_error("fork");
        }
    }

}
    




