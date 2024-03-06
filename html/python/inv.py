#!/usr/bin/python3
import cgi
import sys


print("Content-Type: text/plain;charset=utf-8")
print()


form = cgi.FieldStorage()


data = form.getvalue('data', '')


reversed_data = data[::-1]


print(reversed_data)