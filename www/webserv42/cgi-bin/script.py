#!/usr/bin/env python3

print("Content-Type: text/plain\n")

# Open a file using a relative path
with open("data.txt", "r") as f:
    content = f.read()

print("Contents of data.txt:")
print(content)
