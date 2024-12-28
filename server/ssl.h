#ifndef SSL_H
#define SSL_H

#include <../openssl/ssl.h>
#include <../openssl/err.h>
#include <../openssl/bio.h>
#include <../openssl/crypto.h>

#define SSL_CERT_FILE "server.crt"  // Path to your SSL certificate
#define SSL_KEY_FILE  "server.key" // Path to your private key
#define SSL_METHOD TLS_server_method() // Protocol (e.g., TLS)

// Initializes the SSL library
void initialize_ssl();

// Creates and configures an SSL context
SSL_CTX *create_ssl_context();

// Configures the SSL context with certificates and keys
void configure_ssl_context(SSL_CTX *ctx);

// Cleans up SSL resources
void cleanup_ssl();

