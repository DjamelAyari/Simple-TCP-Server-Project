#include "server.h"
#include "request.h"

char *method_parse, *path_parse = NULL;
long file_size = 0.

process_get_request(ptr_header)
{
    parse_request(ptr_header);
    if(strstr(path_parse, "/home"))
    {
        
    }
}

process_post_request(ptr_header, ptr_body)
{
    parse_request(ptr_header);
}

parse_request(char *header_pointer)
{
    char *parse_cpy_ptr_header = strdup(ptr_header;)
    if(parse_cpy_ptr_header != NULL)
    {
        method_parse = strtok(get_cpy_ptr_header, " ");
        path_parse = strtok(NULL, " ");
    }
    else if(parse_cpy_ptr_header == NULL)
    {
        fprintf(stdout, "parse_cpy_ptr_header is NULL !\n");
        return(1);
    }

    int i = 0;
	while (path_parse[i] != '\0')
	{
		if (path_parse[i] == '+')
		{
			path_parse[i] = ' ';
		}

		i++;
	}

	printf("Path after replacement: %s\n", path_parse);
}

get_size_file()
{
    FILE *ptr_file;
    ptr_file = fopen("../html/high.html", "r");
    if(!ptr_file)
    {
        const char *not_found_response = 
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";

        SSL_write(ssl, not_found_response, strlen(not_found_response));
        return(1);
    }

    fseek(ptr_file, 0, SEEK_END);
    if(fseek < 0)
    {
        fprintf(stdout, "fseek < 0 !\n");
        fclose(ptr_file);
        return(1);
    }

    file_size = ftell(ptr_file);

    const char *response_header = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Content-Length: %ld\r\n"
    "Connection: close\r\n\r\n",
    file_size;

    SSL_write(ssl, response_header, strlen(response_header));

    char file_data_chunk_send[4096];
    size_t bytes_read;
    while((bytes_read = fread(file_data_chunk_send, 1, sizeof(file_data_chunk_send), ptr_file)) > 0)
    {
        SSL_write(ssl, file_data_chunk_send, bytes_read);
    }

    fclose(ptr_file);
}

