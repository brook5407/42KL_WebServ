#!/usr/bin/python3

import os

user_var = os.environ['USER']
print("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 17\nConnection: keep-alive\n")
print(f"Hello, {user_var}")