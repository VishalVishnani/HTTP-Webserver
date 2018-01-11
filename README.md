ECEN 5273- Network Systems

SIMPLE HTTP WEBSERVER

Author: Vishal Vishnani

Date: 10/22/2017


Description: Simple HTTP Web Server (SHWS) is a web server which hosts image files, html pages and text files. SHWS supports multiple clients at the same time using fork(). It supports two methods: GET and POST. It also has error handling mechanism.


File Structure:

1]server.c - It takes the request from webpage and sends the reply back.
             It handles following errors:
               -400 Bad Request
               -404 Not Found
               -501 Not Implemented
               -500 Internal Server Error

3]ws.conf  - Webserver is configured from the parameters given in this file.
             Parameters like supported extensions, port number.
 
2]Makefile - Makefile to create executable


Execution:

1]First create executable and start the server.
    make server
    ./server

2]You can send request to http://localhost:[port number] from browser

