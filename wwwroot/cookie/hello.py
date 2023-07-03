# #!/usr/bin/python3

from urllib.parse import parse_qs

def parse_urlencoded_formdata(data):
    # Parse the URL-encoded form data string
    parsed_data = parse_qs(data)

    # Convert the parsed data into a dictionary
    form_data = {}
    for key, values in parsed_data.items():
        # If a key has multiple values, store them in a list
        if len(values) > 1:
            form_data[key] = values
        # If a key has a single value, store it as is
        else:
            form_data[key] = values[0]

    return form_data

from http.cookies import SimpleCookie

def parse_http_cookie(cookie_string):
    # Create a SimpleCookie object
    cookie = SimpleCookie()
    cookie.load(cookie_string)

    # Convert the cookie into a dictionary
    cookie_dict = {}
    for key, morsel in cookie.items():
        cookie_dict[key] = morsel.value

    return cookie_dict

import sys
import os

if os.environ["REQUEST_METHOD"] == "POST" :
	form_data = sys.stdin.read()
	print("X-Replace-Session: ID=" + form_data)
	user_data = parse_urlencoded_formdata(form_data)
else :
	cookie_data = os.environ["HTTP_SESSION"]
	cookie_map = parse_http_cookie(cookie_data)
	form_data = cookie_map["ID"]
	user_data = parse_urlencoded_formdata(form_data)

# print("X-Replace-Session: blabla\r")
print("X-Cehck: \r")
print("Content-Type: text/html\r")  # Set the Content-Type header to indicate HTML content
print("\r")  # Print an empty line to indicate the end of the header
print(user_data) 











# # Split the form data into individual key-value pairs
# form_values = form_data.split('&')

# # Process the form values
# name = ''
# age = ''
# for pair in form_values:
#     key, value = pair.split('=')
#     if key == 'name':
#         name = value
#     elif key == 'age':
#         age = value








# user_var = os.environ['USER']
# print("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 17\nConnection: keep-alive\n")

# Read the form data from standard input












print(f"Hello world!")

# import os

# # Retrieve the submitted values from the environment variables
# name = os.environ.get('name', '')
# age = os.environ.get('age', '')

# # Print the submitted values
# print("Content-Type: text/html\r")
# print("\r")
# print("<html>")
# print("<head><title>Submitted Values</title></head>")
# print("<body>")
# print("<h1>Submitted Values</h1>")
# print("<p>Name: {0}</p>".format(name))
# print("<p>Age: {0}</p>".format(age))
# print("</body>")
# print("</html>")




# # Print the submitted values
# print("Content-Type: text/html\r")
# print("\r")
# print("<html>")
# print("<head><title>Submitted Values</title></head>")
# print("<body>")
# print("<h1>Submitted Values</h1>")
# print("<p>Name: {0}</p>".format(name))
# print("<p>Age: {0}</p>".format(age))
# print("</body>")
# print("</html>")

# managed to get age and name sent via post request. now need to extract name and age and construct a proper response header for hello.py.