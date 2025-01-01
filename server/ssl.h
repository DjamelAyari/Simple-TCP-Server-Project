#ifndef SSL_H
#define SSL_H

#include <../openssl/ssl.h>
#include <../openssl/err.h>
#include <../openssl/bio.h>
#include <../openssl/crypto.h>

#define SSL_CERT_FILE "server.crt"  // Path to your SSL certificate
#define SSL_KEY_FILE  "server.key" // Path to your private key
#define SSL_METHOD TLS_server_method() // Protocol (e.g., TLS)

