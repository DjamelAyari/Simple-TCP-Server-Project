void allocate_buffers();

void handle_client_request(SSL *ssl);

void fill_header_buffer(char *request_pointer);

void content_length_extraction(char *header_pointer);

void send_cors_response(SSL *ssl);

void fill_body_buffer(char *header_pointer, int content_length);

void parse_request(char *header_pointer);

void process_get_request(char *header_pointer);

void process_post_request(char *header_pointer, char *body_pointer);

void send_file(SSL *ssl, const char *file_path);


