#include "server.h"
#include "ssl.h"
#include "request.h"

char *method_parse, *path_parse = NULL;
long file_size = 0;

void process_get_request(char *ptr_header)
{
    parse_request(ptr_header);
    if(strstr(path_parse, "/home"))
    {
        snprintf(path_parse, sizeof(path_parse), "../html%s.html");
        send_file(path_parse);
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

send_file()
{
    complite_file_path = 
    FILE *ptr_file;
    ptr_file = fopen(path_parse, "r");
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

    if(fseek(ptr_file, 0, SEEK_END) != 0)
    {
        fprintf(stdout, "fseek < 0 !\n");
        fclose(ptr_file);
        return(1);
    }

    file_size = ftell(ptr_file);
    if (file_size < 0)
    {
        fprintf(stdout, "ftell failed!\n");
        fclose(ptr_file);
        return(1);
    }

    rewind(ptr_file);

    char response_header[256];
    snprintf(response_header, sizeof(response_header),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Content-Length: %ld\r\n"
    "Connection: close\r\n\r\n",
    file_size);

    SSL_write(ssl, response_header, strlen(response_header));

    char file_data_chunk_send[4096];
    size_t bytes_read;
    while((bytes_read = fread(file_data_chunk_send, 1, sizeof(file_data_chunk_send), ptr_file)) > 0)
    {
        SSL_write(ssl, file_data_chunk_send, bytes_read);
    }

    fclose(ptr_file);
}

