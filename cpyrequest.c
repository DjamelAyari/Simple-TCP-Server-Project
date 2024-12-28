#include "server.h"
#include "request.h"

#define INITIAL_BUFFER_SIZE 1024
#define CHUNK_SIZE 1024

handle_client_request(SSL *ssl)
{
    int bytes_received = 0;
    int total_bytes_received = 0;

    char *ptr_request, *ptr_header, *ptr_body = NULL;
    
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
        }

        fprintf(stdout, "***BYTES RECEIVED = %d***\n", bytes_received);
    }

    if (bytes_received <= 0)
    {
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

    fprintf(stdout, "***TOTAL BYTES RECEIVED = %d***\n", total_bytes_received);

    fprintf(stdout, "***RAW REQUEST: %s***\n", ptr_request);

    fprintf(stdout, "***COPYING THE HEADER IN THE HEADER'S BUFFER***\n");

    char *ptr_header_end = strstr(ptr_request, "\r\n\r\n");

    if(ptr_header_end != NULL)
    {
        int header_len = ptr_header_end - ptr_request + 4;
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
        }

        strncpy(ptr_header, ptr_request, header_len);
        ptr_header[header_len] = '\0';

        fprintf(stdout, "***COPYING THE BODY IN THE BODY'S BUFFER IF BODY EXIST***\n");

        char *cpy_ptr_header = strdup(ptr_header);
        int content_length = 0;
        char *method = strtok(cpy_ptr_header, " ");
        cpy_ptr_header = NULL;
        if(method != NULL && strcmp(method, "POST") == 0)
        {
            char content_length_char = strstr(ptr_header, "Content-Length: ") + 16;
            if(content_length_char != NULL)
            {
                char content_length_char_end = strstr(content_length_char, "\r\n");
                if (content_length_char_end != NULL)
				{
					*content_length_char_end = '\0';
					content_length = atoi(content_length_char);
					*content_length_char_end = '\r';  // Restore original character
				}
                else
                {
                    printf("content_length_char_end is NULL !!!\n");
                }
            }
            else
            {
                printf("content_length_char is NULL !!!\n");
            }

            if(content_length > 0)
            {
                int body_len = total_bytes_received - header_len;
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
                }

                strncpy(ptr_body, ptr_request, body_len);
                ptr_body[body_len] = '\0';
            }
            else
            {
                printf("Content-Length is not superior to 0 !!!\n");
            }
        }
    }
    else
    {
        printf("Error: Header not found!\n");
        free(ptr_request);
        free(ptr_header);
        free(ptr_body);
        return(1);
    }

    fprintf(stdout, "***PARSING THE HEADER***\n");
    //1. Parse the HTTPS header.
    //2. Check for the Origin header.
    //3. If the Origin header is present and the request is cross-origin, handle CORS.
    //4. If the method is OPTIONS, respond with the appropriate CORS headers and skip further processing.
    //5. Handle the request (GET or POST).

    /*HTTP/1.1 200 OK
    Access-Control-Allow-Origin: http://anotherdomain.com
    Access-Control-Allow-Methods: GET, POST
    Access-Control-Allow-Headers: Content-Type
    Access-Control-Allow-Credentials: true*/

    /*Example of the Server's Response to Preflight Request:
    HTTP/1.1 200 OK
    Access-Control-Allow-Origin: http://mywebsite.com
    Access-Control-Allow-Methods: GET, POST
    Access-Control-Allow-Headers: Content-Type
    Access-Control-Allow-Credentials: true
    Access-Control-Max-Age: 86400*/
    if(method == )
    

    

    free(ptr_request);
    free(ptr_header);
    free(ptr_body);
}