#!/usr/bin/python3
import cgi
import os

print("Content-type: text/html\n")
print("<html>")
print("<head><title>Hello World CGI</title></head>")
print("<body>")
print("<h1>Hello World!</h1>")
print("<p>Query string: " + os.environ['QUERY_STRING'] + "</p>")
print("</body>")
print("</html>")
