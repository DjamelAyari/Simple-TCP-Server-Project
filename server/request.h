void handle_client_request(SSL *ssl);

void fill_header_buffer(char *request_pointer);

void content_length_extraction(char *header_pointer);

void fill_body_buffer(char *header_pointer);


