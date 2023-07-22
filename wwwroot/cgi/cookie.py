import os
import sys

print("Content-Type: text/html\r")
print("Set-Cookie: name=John Python\r")
print("\r")
print("<html><head><title>Cookie Set - Python CGI</title></head>")
print("<body>")
print("<h1>Cookie Set - Python CGI</h1>")
print(os.environ)
print(sys.stdin.read())
print("<form method=\"POST\" >")
print("<input type=\"text\" name=\"cookie_name\" />")
print("<input type=\"text\" name=\"cookie_value\" />")
print("<input type=\"submit\" value=\"Submit\" />")
print("</form>")
print("<form method=\"POST\" >")

print("<input type=\"submit\" name=\"Delete\" value=\"Delete\" />")
print("</form>")
print("Hello World!</body></html>")

#print("Content-Type: text/html\r")