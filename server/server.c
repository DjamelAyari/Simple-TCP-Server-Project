#include "server.h"
#include "ssl.h"
#include "request.h"

int main()
{
    #if defined (_WIN32)
    WSADATA d;
    fprintf(stdout, "Winsock initialization...\n");
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        fprintf(stderr, "Failed to initialize: %s\n", strerror(errno));
        return(1);
    }
    else
    {
        fprintf(stdout, "Winsock initialion ok !\n");
    }
    #endif

    fprintf(stdout, "SSL librairy initialization...\n");
    if (SSL_library_init() != 1)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "Error initializing SSL library: %s\n", ERR_error_string(err, NULL));
        return(1);
    }

    fprintf(stdout, "SSL diagnostic stetting up...\n");
    SSL_load_error_strings();
    if (ERR_peek_error() != 0)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "Error setting up diagnostic strings: %s\n", ERR_error_string(err, NULL));
        return(1);
    }

    fprintf(stdout, "SSL encryption tools stetting up...\n");
    if (OpenSSL_add_ssl_algorithms() != 1)
    {
        unsigned long err = ERR_get_error();
        fprintf(stderr, "Error setting up encryption tools: %s\n", ERR_error_string(err, NULL));
        return(1);
    }

    fprintf(stdout, "SSL context creation...\n");
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) 
    {
        fprintf(stderr, "Unable to create SSL context.\n");
        ERR_print_errors_fp(stderr);
        return(1);
    }

    // Set protocol versions
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);

    printf("Using OpenSSL version: %s\n", OpenSSL_version(OPENSSL_VERSION));

    fprintf(stdout, "SSL certificat loading...\n");
    fprintf(stderr, "Attempting to load certificate from: %s\n", SSL_CERT_FILE);
    if (SSL_CTX_use_certificate_file(ctx, SSL_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
    {
        fprintf(stderr, "Error loading SSL certificate.\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return(1);
    }

    fprintf(stdout, "SSL key loading...\n");
    fprintf(stderr, "Attempting to load key from: %s\n", SSL_KEY_FILE);
    if (SSL_CTX_use_PrivateKey_file(ctx, SSL_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
    {
        fprintf(stderr, "Error loading SSL key.\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return(1);
    }

    if (!SSL_CTX_check_private_key(ctx))
    {
        fprintf(stderr, "Private key does not match the certificate public key\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 1;
    }

    
    fprintf(stdout, "SSL initialization completed successfully.\n");
    fprintf(stdout, "Ready to use socket API.\n");

    fprintf(stdout, "Configuration of the address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *bind_address;
    int result = getaddrinfo("localhost", "443", &hints, &bind_address);//127.0.0.1
    if (result != 0)
    {
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(result));
        return(1);
    }
    fprintf(stdout, "Address configuration ok !\n");


    fprintf(stdout, "Creating the socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if(!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() failed: %s\n", strerror(GETSOCKETERRNO()));
        freeaddrinfo(bind_address);
        SSL_CTX_free(ctx);
        CLOSESOCKET(socket_listen);
        return(1);
    }
    fprintf(stdout, "Socket creating ok !\n");

    fprintf(stdout, "Binding the socket to the address...\n");
    if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen) == -1)
    {
        fprintf(stderr, "bind() failed: %s\n", strerror(GETSOCKETERRNO()));
        freeaddrinfo(bind_address);
        SSL_CTX_free(ctx);
        CLOSESOCKET(socket_listen);
        return(1);
    }
    freeaddrinfo(bind_address);
    fprintf(stdout, "Binding ok !\n");

    fprintf(stdout, "Listenning...\n");
    if (listen(socket_listen, 5) < 0)
    {
        fprintf(stderr, "listen() failed: %s\n", strerror(GETSOCKETERRNO()));
        SSL_CTX_free(ctx);
        CLOSESOCKET(socket_listen);
        return(1);
    }
    fprintf(stdout, "Listen ok !\n");

    fprintf(stdout, "Creation of the fd_set master...\n");
    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    fprintf(stdout, "Waiting for connexion...\n");

    SOCKET i;
    while(1)
    {
        fd_set reads;
        reads = master;
        if(select(max_socket + 1, &reads, 0, 0, 0) < 0) //select < 0 car si la valeur de retour de select est < 0 cela veut dire qu'il y a eu une erreur.
        {
            fprintf(stderr, "select() failed: %s\n", strerror(GETSOCKETERRNO()));
            SSL_CTX_free(ctx);
            CLOSESOCKET(socket_listen);
            return(1);
        }
        else
        {
            fprintf(stdout, "Select ok !\n");
        }

        for(i = 1; i <= max_socket; i++)// i commence à 1, tant que i est < ou = à maw_socket, il faut incrémenter i de 1
        {
            if(FD_ISSET(i, &reads))// si le i prêt à être pris en charge fait déjà partie du set reads il faut exectuer ce qu'il y a dans le if
            {

                if(i == socket_listen)//si i == socket_listen veut dire que si un client demande à ce connecter, il s'est signalé au server et donc le server va pouvoir le gérer, il n'est pas encore intégré dans le set, il n'est pas encore rentré dans le server
                {
                    struct sockaddr_storage client_address;// création d'une structure de type sockaddr_storage dénomé client_address pour pouvoir stocker l'adresse du client
                    socklen_t client_len = sizeof(client_address);//création d'un variable de type socklen_t pour stocker la taille de l'adresse du client

                    SOCKET socket_client = accept(socket_listen, (struct sockaddr*)&client_address, &client_len);// acceptation de la demande de connexion du client
                    if(!ISVALIDSOCKET(socket_client))// si la valeur de retour de accept correspond à un erreur, il faut lever l'erreur, et donc l'afficher sur la console
                    {
                        fprintf(stderr, "select() failed ! (%d)\n", GETSOCKETERRNO());
                        SSL_CTX_free(ctx);
                        CLOSESOCKET(socket_listen);
                        return(1);
                    }

                    FD_SET(socket_client, &master);// une fois le client accepté, son fd est mit dans le set master.
                    if(socket_client > max_socket)
                    {
                        max_socket = socket_client;
                    }

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
                    fprintf(stdout, "New connexion from %s\n", address_buffer);
                }
                else
                {
                    printf("Creating new SSL object for client socket: %d\n", i);
                    SSL *ssl = SSL_new(ctx);// Create SSL object
                    if (!ssl)
                    {
                        fprintf(stderr, "SSL_new failed.\n");
                        fprintf(stderr, "SSL_new() failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
                        SSL_CTX_free(ctx);
                        CLOSESOCKET(i);
                        FD_CLR(i, &master);
                        continue;
                    }
                    fprintf(stdout, "SSL_new() ok !\n");


                    SSL_set_fd(ssl, i);// Attach to socket_client (i)
                    if (SSL_set_fd(ssl, i) != 1)
                    {
                        fprintf(stderr, "SSL_set_fd() failed.\n");
                        fprintf(stderr, "SSL_set_fd() failed for socket %d: %s\n", i, ERR_error_string(ERR_get_error(), NULL));
                        SSL_free(ssl);
                        CLOSESOCKET(i);
                        FD_CLR(i, &master);
                        continue;
                    }
                    fprintf(stdout, "SSL_set_fd() ok !\n");

                    if (SSL_accept(ssl) <= 0)
                    {
                        fprintf(stderr, "SSL_accept failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
                        SSL_free(ssl);
                        CLOSESOCKET(i);
                        FD_CLR(i, &master);
                        continue;
                    }
                    fprintf(stdout, "SSL_set_fd() ok !\n");
                    
                    fprintf(stdout, "Entering the handle_client_request()...\n");
                    handle_client_request(ssl);
                    SSL_shutdown(ssl);
                    fprintf(stdout, "SSL_shutdown() ok !\n");
                    SSL_free(ssl);
                    fprintf(stdout, "SSL_free() ok !\n");
                    CLOSESOCKET(i);
                    fprintf(stdout, "CLOSESOCKET() ok !\n");
                    FD_CLR(i, &master);
                    fprintf(stdout, " FD_CLR(() ok !\n");
                }
            }//if FD_ISSET
        }//for i to max_socket
    }//while end

    fprintf(stdout, "Closing listenning socket...\n");
    CLOSESOCKET(socket_listen);
    fprintf(stdout, "listen socket closed !\n");
    SSL_CTX_free(ctx);
    fprintf(stdout, "SSL_CTX_free() ok !\n");
    EVP_cleanup();
    fprintf(stdout, "EVP_cleanup() ok !\n");
    CRYPTO_cleanup_all_ex_data();
    fprintf(stdout, "CRYPTO_cleanup_all_ex_data() ok !\n");
    ERR_free_strings();
    fprintf(stdout, "ERR_free_strings() ok !\n");
    FD_CLR(i, &master);
    fprintf(stdout, "FD_CLR() ok !\n");


    #if defined(_WIN32)
        WSACleanup();
    #endif

    fprintf(stdout, "WSACleanup() ok !\n");
    fprintf(stdout, "Finished, End Of server.c File !\n");

    return(0);
}

