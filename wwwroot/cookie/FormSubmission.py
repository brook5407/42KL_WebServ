# print("Content-Type: text/html\r")  # Set the Content-Type header to indicate HTML content
# print("\r")  # Print an empty line to indicate the end of the header

# # HTML content
# print("<html>")
# print("<head><title>User Information</title></head>")
# print("<body>")
# print("<h1>User Information</h1>")
# print('<form method="POST" action="http://localhost:8080/cookie/hello.py">')

# # Input fields for name and age
# print('<label for="name">Name:</label>')
# print('<input type="text" id="name" name="name"><br>')

# print('<label for="age">Age:</label>')
# print('<input type="text" id="age" name="age"><br>')

# # Submit button
# print('<input type="submit" value="Submit">')
# print('</form>')

# print("</body>")
# print("</html>")

import os

print("Content-Type: text/html\r")
# if 'HTTP_COOKIE' not in os.environ:
# print("Set-Cookie: name=John Python\r")
print("\r")

print("<html>")
print("<head><title>User Information</title></head>")
print("<body>")
print("<h1>User Information</h1>")
print('<form method="POST" action="http://localhost:8080/cookie/hello.py" xonsubmit="sendData(event)">')

print('<label for="name">Name:</label>')
print('<input type="text" id="name" name="name"><br>')

print('<label for="age">Age:</label>')
print('<input type="text" id="age" name="age"><br>')

print('<input type="submit" value="Submit">')
print('</form>')

# JavaScript code to handle form submission and send the POST request
print('<script>')
print('function sendData(event) {')
print('  event.preventDefault();')  # Prevent form submission
print('  var name = document.getElementById("name").value;')
print('  var age = document.getElementById("age").value;')
print('  var xhr = new XMLHttpRequest();')
print('  xhr.open("POST", "http://localhost:8080/cookie/hello.py");')
print('  xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");')
print('  xhr.setRequestHeader("name", name);')
print('  xhr.setRequestHeader("age", age);')
print('  xhr.onload = function() {')
print('    if (xhr.status === 200) {')
print('      console.log("POST request successful!");')
print('    } else {')
print('      console.log("POST request failed.");')
print('    }')
print('  };')
print('  xhr.send();')
print('}')
print('</script>')

print("</body>")
print("</html>")
