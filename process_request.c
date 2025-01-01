#include "server.h"
#include "request.h"

process_get_request(ptr_header)
{
    parse_request(ptr_header);
    if(strstr(path_parse, "/high"))
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

send_file(ssl, )
{

}