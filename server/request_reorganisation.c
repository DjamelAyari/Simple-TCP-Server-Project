#include "server.h"
#include "request.h"

#define INITIAL_BUFFER_SIZE 1024
#define CHUNK_SIZE 1024

int bytes_received = 0;
int total_bytes_received = 0;
int header_len = 0;
int content_length = 0;
char *method;
char content_length_char;
char content_length_char_end;
int body_len;

char *ptr_request, *ptr_header, *ptr_body = NULL;
char *ptr_header_end = NULL;

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

handle_client_request(SSL *ssl)
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
                free(ptr_request);
                free(ptr_header);
                free(ptr_body);
                return(1);
            }
            ptr_request = temp_request;
            free(temp_request);
            temp_request = NULL;
        }

        fprintf(stdout, "***BYTES RECEIVED = %d***\n", bytes_received);
    }

    if (bytes_received <= 0)
    {
        fprintf("bytes_received <= 0\n");
        
        int ssl_error = SSL_get_error(ssl, bytes_received);
        fprintf(stderr, "SSL_read failed with error: %d\n", ssl_error);

        if (ssl_error == SSL_ERROR_ZERO_RETURN)
        {
            fprintf(stderr, "Connection closed by the client.\n");
            free(ptr_request);
            free(ptr_header);
            free(ptr_body);
            return(1);
        }
        else if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE)
        {
            fprintf(stderr, "SSL operation not complete. Try again later.\n");
            free(ptr_request);
            free(ptr_header);
            free(ptr_body);
            return(1);
        }
        else
        {
            fprintf(stderr, "SSL error occurred.\n");
            free(ptr_request);
            free(ptr_header);
            free(ptr_body);
            return(1);
        }

        free(ptr_request);
        free(ptr_header);
        free(ptr_body);
        return (1);
    }
    ptr_request[bytes_received] = '\0';
}

fprintf(stdout, "***TOTAL BYTES RECEIVED = %d***\n", total_bytes_received);

fprintf(stdout, "***RAW REQUEST: %s***\n", ptr_request);

fprintf(stdout, "***COPYING THE HEADER IN THE HEADER'S BUFFER***\n");

fill_header_buffer(ptr_request)
{
    ptr_header_end = strstr(ptr_request, "\r\n\r\n");

    if(ptr_header_end != NULL && strncmp(ptr_header_end, "\r\n\r\n", 3) == 0)
    {
        header_len = ptr_header_end - ptr_request + 4;
        if(header_len > INITIAL_BUFFER_SIZE)
        {
            char *temp_header = realloc(ptr_header, header_len);
            if(temp_header == NULL)
            {
                printf("Memory reallocation for header failed!\n");
                free(ptr_request);
                free(ptr_header);
                free(ptr_body);
                return(1);
            }
            ptr_header = temp_header;
            free(temp_header);
            temp_header = NULL;
        }

        strncpy(ptr_header, ptr_request, header_len);
        ptr_header[header_len] = '\0';
    }
    else
    {
        fprintf(stdout, "ptr_header_end is NULL or it's value is not \r\n\r\n !\n");
        free(ptr_request);
        free(ptr_header);
        free(ptr_body);
        return(1);
    }
}

fprintf(stdout, "***COPYING THE BODY IN THE BODY'S BUFFER IF BODY EXIST***\n");

fill_body_buffer()
{
    char *cpy_ptr_header = strdup(ptr_header);
    method = strtok(cpy_ptr_header, " ");
    free(cpy_ptr_header);
    cpy_ptr_header = NULL;

    if(method != NULL && strncmp(method, "POST", 3) == 0)//Take of the possibility that method could be GET
    {
        content_length_char = strstr(ptr_header, "Content-Length: ") + 16;
        if(content_length_char != NULL && strncmp(content_length_char, "Content-Length: ", 15) == 0)
        {
            content_length_char_end = strstr(content_length_char, "\r\n");
            if (content_length_char_end != NULL && strncmp(content_length_char_end, "\r\n", 1) == 0)
			{
				*content_length_char_end = '\0';
				content_length = atoi(content_length_char);
				*content_length_char_end = "\r\n";  // Restore original character
			}
            else
            {
                printf("content_length_char_end is NULL !!!\n");
            }
        }
        else
        {
            fprintf(stdout, "content_length_char is NULL or it's value is not Content-Length: !\n");
            return(1);
        }

        if(content_length > 0)
        {
            body_len = total_bytes_received - header_len;
            if(body_len > INITIAL_BUFFER_SIZE)
            {
                char *temp_body = realloc(ptr_body, body_len);
                if(temp_body == NULL)
                {
                    printf("Memory reallocation for body failed!\n");
                    free(ptr_request);
                    free(ptr_header);
                    free(ptr_body);
                    return(1);
                }
                ptr_body = temp_body;
                free(temp_body);
            }

            strncpy(ptr_body, ptr_request, body_len);
            ptr_body[body_len] = '\0';
        }
        else
        {
            printf("Content-Length is not superior to 0 !!!\n");
        }
    }
    else
    {
        fprintf(stdout, "method is NULL or it's value is not POST ! \n");
        return(1);
    }
    

}

