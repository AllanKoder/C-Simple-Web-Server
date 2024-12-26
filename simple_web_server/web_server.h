void sigchld_handler(int s);
int create_server_socket();
void configure_socket(int server_socket);
void handle_client(int client_socket);
void accept_connections(int server_socket);
