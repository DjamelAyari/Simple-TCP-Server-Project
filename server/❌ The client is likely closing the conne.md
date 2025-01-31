❌ The client is likely closing the connection improperly.

Since SSL_pending(ssl) returns 0 and SSL_read() gives you only zeros, it strongly suggests that:

The client has closed the connection properly after the first request.
Your server is trying to read from an already closed connection (which might still exist at the OS level but has no more data to read).

Possible Cause: SSL/TLS session state may not be managed correctly between requests. The browser and server might be maintaining a session incorrectly, or the state could be reset, leading to unexpected behavior when trying to read subsequent requests.

the connection is not handled properly by the server after redirection, the browser could fail to establish the connection properly. Another possibility is that the server doesn't properly reset or maintain the session when responding to the redirected request, causing issues with subsequent communication.


after post ssl has issues

By ensuring that the server properly handles SSL session management and connection state between requests, you should be able to resolve the issue. The fact that it works after a restart indicates that the server needs to better handle session persistence and reset between requests.

Ensure that the SSL session is properly RESET after the POST request, or force a new SSL session for each new request to avoid session persistence issues.

Key Areas to Investigate:
Session Management:

Ensure SSL sessions are properly reset between requests or ensure SSL session reuse is correctly implemented.
Connection Handling:

Make sure the server closes connections properly after the POST request and forces the client to open a new connection for subsequent requests.
SSL/TLS Buffer Management:

Check the server's handling of SSL buffers to ensure they are correctly flushed and reset after the POST request.
Redirect Handling:

Verify that any redirect responses are correctly processed, and the server properly closes the connection after sending a redirect.
Postman and Curl Tests:

Test the sequence with Postman or Curl to rule out client-side issues, especially regarding session management or connection reuse.
By focusing on how the server handles SSL session management, connection state, and buffer clearing between requests, you should be able to pinpoint the issue and resolve the problem with receiving invalid (zeroed) data after the POST request.

