#include "server.h"
#include "request.h"
#include "ssl.h"
#include <stdarg.h>

#define INITIAL_BUFFER_SIZE 1024
#define CHUNK_SIZE 1024

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
            free(*current);      // Free the memory.
            *current = NULL;     // Set the pointer to NULL.
        }

        current = va_arg(args, void **);  // Get the next argument.
    }

    va_end(args);
}


ptr_request = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
if(ptr_request == NULL)
{
    printf("Memory allocation for request failed !\n");
    return(1);
}   
memset(ptr_request, 0, INITIAL_BUFFER_SIZE);

ptr_header = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
if(ptr_header == NULL)
{
    printf("Memory allocation for header failed !\n");
    free(ptr_request);
    return(1);
}   
memset(ptr_header, 0, INITIAL_BUFFER_SIZE);

ptr_body = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
if(ptr_body == NULL)
{
    printf("Memory allocation for body failed !\n");
    free(ptr_request);
    free(ptr_header);
    return(1);
}   
memset(ptr_body, 0, INITIAL_BUFFER_SIZE);

void handle_client_request(SSL *ssl)
{
    while((bytes_received = SSL_read(ssl, ptr_request+total_bytes_received, CHUNK_SIZE)) > 0)
    {
        //Prendre en charge CORS
        //Prendre en charge les réponses HTTP (200 Ok...)
        total_bytes_received += bytes_received;

        if(total_bytes_received >= INITIAL_BUFFER_SIZE)
        {
            char *temp_request = realloc(ptr_request, total_bytes_received + CHUNK_SIZE);
            if(temp_request == NULL)
            {
                printf("Memory reallocation for request failed!\n");
                free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
                return(1);
            }
            ptr_request = temp_request;
        }
        loop_nbr += 1;
        fprintf(stdout, "***BYTES RECEIVED DURING LOOP:%d = %d***\n", loop_nbr, bytes_received);
        fprintf(stdout, "***TOTAL BYTES RECEIVED DURING LOOP:%d = %d***\n", loop_nbr, total_bytes_received);
    }

    if (bytes_received <= 0)
    {
        fprintf(stdout, "WARNING -> bytes_received <= 0\n");
        
        int ssl_error = SSL_get_error(ssl, bytes_received);
        fprintf(stderr, "SSL_read failed with error: %d\n", ssl_error);

        if (ssl_error == SSL_ERROR_ZERO_RETURN)
        {
            fprintf(stderr, "Connection closed by the client.\n");
            free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
            return(1);
        }
        else if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE)
        {
            fprintf(stderr, "SSL operation not complete. Try again later.\n");
            free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
            return(1);
        }
        else
        {
            fprintf(stderr, "SSL error occurred.\n");
            free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
            return(1);
        }

        free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
        return (1);
    }
    ptr_request[bytes_received] = '\0';
}

void send_cors_response(SSL *ssl)
{
    if (strncmp(ptr_request, "OPTIONS", 7) == 0)
    {
        const char *response =
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "\r\n";

        SSL_write(ssl, response, strlen(response));
    }
}

handle_client_request(SSL *ssl);

fprintf(stdout, "***TOTAL BYTES RECEIVED AFTER WHILE LOOP END = %d***\n", total_bytes_received);

fprintf(stdout, "***RAW REQUEST: %s***\n", ptr_request);

fprintf(stdout, "***COPYING THE HEADER IN THE HEADER'S BUFFER***\n");

void fill_header_buffer(ptr_request)
{
    char *ptr_header_end = strstr(ptr_request, "\r\n\r\n");

    if(ptr_header_end != NULL && strncmp(ptr_header_end, "\r\n\r\n", 4) == 0)
    {
        header_len = ptr_header_end - ptr_request + 4;
        if(ptr_header + header_len > INITIAL_BUFFER_SIZE)
        {
            char *temp_header = realloc(ptr_header, header_len);
            if(temp_header == NULL)
            {
                printf("Memory reallocation for header failed!\n");
                free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
                return(1);
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
        return(1);
    }
    else
    {
        fprintf(stdout, "ptr_header_end value is not '\r\n\r\n' !\n");
        free_and_null((void **)&ptr_request, (void **)&ptr_header, (void **)&ptr_body, NULL);
        return(1);
    }
}

fprintf(stdout, "***SEARCH AND EXTRACTION OF CONTENT LENGTH IF IT EXIST***\n");

void content_length_extraction(ptr_header)
{
    char *cpy_ptr_header = strdup(ptr_header);
    method = strtok(cpy_ptr_header, " ");
    
    if(method == NULL)
    {
        fprintf(stdout, "The method is NULL !\n");
        return(1);
    }
    
    if(strcmp(method, "GET") == 0)
    {
        fprintf(stdout, "The method is GET !\n");
        process_get_request(ptr_header);
        return(1);
    }
   
    if(strcmp(method, "POST") != 0)
    {
        fprintf(stdout, "The method is not POST !\n");
        fprintf(stdout, "The method is %s !\n", method);
        return(1);
    }

    content_length_char = strstr(ptr_header, "Content-Length: ") + 16;

    if(content_length_char == NULL)
    {
        fprintf(stdout, "The content_length_char is NULL !\n");
        return(1);
    }

    if(! isdigit(*content_length_char))
    {
        fprintf(stdout, "The value of content_length_char is not a number !\n");
        fprintf(stdout, "The value of content_length_char is %s !\n", *content_length_char);
        return(1);
    }

    content_length_char_end = strstr(content_length_char, "\r\n");
    
    if(content_length_char_end == NULL)
    {
        fprintf(stdout, "The content_length_char_end is NULL !\n");
        return(1);
    }

    if(strncmp(content_length_char_end, "\r\n", 1) != 0)
    {
        fprintf(stdout, "The content_length_char_end value is not '\r\n' !\n");
        fprintf(stdout, "The content_length_char_end value is %s !\n", *content_length_char_end);
        return(1);
    }

    *content_length_char_end = '\0';
	content_length = atoi(content_length_char);
	*content_length_char_end = '\r';
}

fprintf(stdout, "***COPYING THE BODY IN THE BODY'S BUFFER IF BODY EXIST***\n");

void fill_body_buffer(ptr_header, content_length)
{

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
                return(1);
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

}

process_post_request(ptr_header, ptr_body);

