#include "server.h"
#include "ssl.h"
#include "request.h"
#include <stdarg.h>

#define INITIAL_BUFFER_SIZE 1024
#define CHUNK_SIZE 1024

/*#define TIMEOUT_SEC 120  // Timeout in seconds
time_t start_time, current_time;*/

int bytes_received = 0;
int total_bytes_received = 0;
int loop_nbr = 1;
int header_len = 0;
int content_length = 0;
char *method = NULL;
char *content_length_char = NULL;
char *content_length_char_end = NULL;
int body_len;

char *ptr_request, *ptr_header, *ptr_body = NULL;


void free_and_null(void **first, ...)
{   
    va_list args;
    void **current = first;

    va_start(args, first);

    while (current != NULL)
    {
        if (*current != NULL)
        {
            free(*current);
            *current = NULL;
        }

        current = va_arg(args, void **);
    }

    va_end(args);
}

void allocate_buffers()
{
    ptr_request = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    if(ptr_request == NULL)
    {
        printf("Memory allocation for request failed !\n");
        return;
    }   
    memset(ptr_request, 0, INITIAL_BUFFER_SIZE);

    ptr_header = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    if(ptr_header == NULL)
    {
        printf("Memory allocation for header failed !\n");
        free(ptr_request);
        return;
    }   
    memset(ptr_header, 0, INITIAL_BUFFER_SIZE);

    ptr_body = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    if(ptr_body == NULL)
    {
        printf("Memory allocation for body failed !\n");
        free(ptr_request);
        free(ptr_header);
        return;
    }   
    memset(ptr_body, 0, INITIAL_BUFFER_SIZE);
}

void handle_client_request(SSL *ssl)
{
    fprintf(stdout, "Entered inside the handle_client_request()\n");
    
    allocate_buffers();
    
    while((bytes_received = SSL_read(ssl, ptr_request+total_bytes_received, CHUNK_SIZE)) > 0 && !strstr(ptr_request, "body_end"))
    {
        fprintf(stdout, "***!!!!!!!!!!!!!!!!!:%d***\n", bytes_received);
        fprintf(stdout, "***!!!!!!!!!!!!!!!!!:%d***\n", total_bytes_received);
        //total_bytes_received += bytes_received;

        printf("BLABLABLA\n");
        if(total_bytes_received >= INITIAL_BUFFER_SIZE)
        {
            char *temp_request = realloc(ptr_request, total_bytes_received + CHUNK_SIZE);
            if(temp_request == NULL)
            {
                printf("Memory reallocation for request failed!\n");
                free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
                return;
            }
            ptr_request = temp_request;
            fprintf(stdout, "Reallocated buffer to %d bytes.\n", total_bytes_received + CHUNK_SIZE);
        }
        total_bytes_received += bytes_received;
        fprintf(stdout, "***BYTES RECEIVED DURING LOOP:%d = %d***\n", loop_nbr, bytes_received);
        fprintf(stdout, "***TOTAL BYTES RECEIVED DURING LOOP:%d = %d***\n", loop_nbr, total_bytes_received);
        
        loop_nbr++;

        // Check timeout: if elapsed time exceeds the timeout, break the loop
        /*time(&current_time);  // Get the current time
        if (difftime(current_time, start_time) >= TIMEOUT_SEC) {
            printf("Timeout reached. Exiting the loop.\n");
            break;  // Exit loop after timeout
        }*/

    }

    printf("***RAW REQUEST: %s***\n", ptr_request);

    if (bytes_received <= 0)
    {
        fprintf(stdout, "WARNING -> bytes_received <= 0\n");
        
        int ssl_error = SSL_get_error(ssl, bytes_received);
        fprintf(stderr, "SSL_read failed with error: %d\n", ssl_error);

        if (ssl_error == SSL_ERROR_ZERO_RETURN)
        {
            fprintf(stderr, "Connection closed by the client.\n");
            free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
            return;
        }
        else if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE)
        {
            fprintf(stderr, "SSL operation not complete. Try again later.\n");
            free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
            return;
        }
        else
        {
            fprintf(stderr, "SSL error occurred.\n");
            free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
            return;
        }

        free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
        return;
    }

    //ptr_request[bytes_received] = '\0';
    //ptr_request[total_bytes_received +1] = '\0';

    send_cors_response(ssl);
    printf("TEST!!!\n");
}

void send_cors_response(SSL *ssl)
{
    fprintf(stdout, "Entered inside the send_cors_response()\n");

    if (strncmp(ptr_request, "OPTIONS", 7) == 0)
    {
        const char *response =
            "HTTP/1.1 204 No Content\r\n"
            "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "\r\n";

        SSL_write(ssl, response, strlen(response));
    }

    //rerun_handle_client_request(ssl);
    fill_header_buffer(ssl, ptr_request);
}

/*void rerun_handle_client_request(SSL *ssl)
{
    fprintf(stdout, "Re run the handle_client_request()\n");
    handle_client_request(ssl);
    fprintf(stdout, "Entered inside the fill_header_buffer()\n");
    fill_header_buffer(ptr_request);
}*/

void fill_header_buffer(SSL *ssl, char *ptr_request)
{
    fprintf(stdout, "Entered inside the fill_header_buffer()\n");

    printf("***TOTAL BYTES RECEIVED AFTER WHILE LOOP END = %d***\n", total_bytes_received);

    printf("***RAW REQUEST: %s***\n", ptr_request);

    printf("***COPYING THE HEADER IN THE HEADER'S BUFFER***\n");
    
    char *ptr_header_end = strstr(ptr_request, "\r\n\r\n");

    if(ptr_header_end != NULL && strncmp(ptr_header_end, "\r\n\r\n", 4) == 0)
    {
        header_len = ptr_header_end - ptr_request + 4;
        if(header_len > INITIAL_BUFFER_SIZE)
        {
            char *temp_header = realloc(ptr_header, header_len);
            if(temp_header == NULL)
            {
                printf("Memory reallocation for header failed!\n");
                free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
                return;
            }
            ptr_header = temp_header;
        }

        strncpy(ptr_header, ptr_request, header_len);
        ptr_header[header_len] = '\0';
    }
    else if(ptr_header_end == NULL)
    {
        fprintf(stdout, "ptr_header_end is NULL !\n");
        free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
        return;
    }
    else
    {
        fprintf(stdout, "ptr_header_end value is not '\r\n\r\n' !\n");
        free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
        return;
    }

    printf("***RAW HEADER: %s***\n", ptr_header);

    content_length_extraction(ssl, ptr_header);
}

void content_length_extraction(SSL *ssl, char *ptr_header)
{
    fprintf(stdout, "Entered inside the content_length_extraction()\n");
    
    printf("***SEARCH AND EXTRACTION OF CONTENT LENGTH IF IT EXIST***\n");
    
    char *cpy_ptr_header = strdup(ptr_header);
    method = strtok(cpy_ptr_header, " ");
    
    if(method == NULL)
    {
        fprintf(stdout, "The method is NULL !\n");
        return;
    }
    
    if(strcmp(method, "GET") == 0)
    {
        fprintf(stdout, "The method is GET !\n");
        process_get_request(ssl, ptr_header);
        return;
    }
   
    if(strcmp(method, "POST") != 0)
    {
        fprintf(stdout, "The method is not POST !\n");
        fprintf(stdout, "The method is %s !\n", method);
        return;
    }

    content_length_char = strstr(ptr_header, "Content-Length: ") + 16;

    if(content_length_char == NULL)
    {
        fprintf(stdout, "The content_length_char is NULL !\n");
        return;
    }

    if(! isdigit(*content_length_char))
    {
        printf("The value of content_length_char is not a number !\n");
        printf("The value of content_length_char is %d !\n", *content_length_char);
        return;
    }

    content_length_char_end = strstr(content_length_char, "\r\n");
    
    if(content_length_char_end == NULL)
    {
        fprintf(stdout, "The content_length_char_end is NULL !\n");
        return;
    }

    if(strncmp(content_length_char_end, "\r\n", 1) != 0)
    {
        fprintf(stdout, "The content_length_char_end value is not '\r\n' !\n");
        fprintf(stdout, "The content_length_char_end value is %d !\n", *content_length_char_end);
        return;
    }

    *content_length_char_end = '\0';
	content_length = atoi(content_length_char);
	*content_length_char_end = '\r';

    ptr_request[total_bytes_received + content_length+1] = '\0';

    fill_body_buffer(ssl, ptr_header, content_length);
}

void fill_body_buffer(SSL *ssl, char *ptr_header, int content_length)
{
    printf("***COPYING THE BODY IN THE BODY'S BUFFER IF BODY EXIST***\n");
    
    if(content_length > 0)
    {
        body_len = total_bytes_received - header_len;
        if(body_len > INITIAL_BUFFER_SIZE)
        {
            char *temp_body = realloc(ptr_body, body_len);
            if(temp_body == NULL)
            {
                printf("Memory reallocation for body failed!\n");
                free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
                return;
            }
            ptr_body = temp_body;
        }

        strncpy(ptr_body, ptr_request, body_len);
        ptr_body[body_len] = '\0';
    }
    else
    {
        printf("Content-Length is not superior to 0 !!!\n");
        printf("Content-Length is %d !!!\n", content_length);
    }

    printf("***RAW BODY: %s***\n", ptr_body);

    process_post_request(ssl, ptr_header, ptr_body);

}

//void process_post_request(SSL *ssl, char *ptr_header, char *ptr_body);

