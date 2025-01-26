void allocate_buffers();

void handle_client_request(SSL *ssl);

void rerun_handle_client_request(SSL *ssl);

void fill_header_buffer(SSL *ssl, char *request_pointer);

void content_length_extraction(SSL *ssl, char *header_pointer);

void send_cors_response(SSL *ssl);

void fill_body_buffer(SSL *ssl, char *header_pointer, int content_length);

void parse_request(char *header_pointer);

void process_get_request(SSL *ssl, char *header_pointer);

void process_post_request(SSL *ssl, char *header_pointer, char *body_pointer);

void send_file(SSL *ssl, char *file_path);

void save_data(SSL *ssl, char *ptr_body);

void thank_you_file(SSL *ssl);


