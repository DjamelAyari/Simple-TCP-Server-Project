#include "server.h"
#include "request.h"

process_get_request(ptr_header)
{
    parse_request(ptr_header)
    //if method == GET and path xxx do xxx
}

process_post_request(ptr_header, ptr_body)
{
    //if method == POST and path xxx do xxx
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
    else if(method_parse == "GET")
    {
        
    }
}
