#!/usr/bin/env python3
import hashlib
import os
import urllib.parse

# CGI-Header: Content-Type MUSS als erste Zeile ausgegeben werden!
print("Content-Type: text/html\n")

# Query-String (vom Webserver oder C++-Wrapper übergeben)
query = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query)

# Username & Passwort extrahieren
username = params.get("username", [""])[0]
password = params.get("password", [""])[0]

# Passwort hashen
hashed_password = hashlib.sha256(password.encode()).hexdigest()

# Ausgabe (für Testzwecke; später nicht Passwort ausgeben!)
print(f"<p>Username: {username}</p>")
print(f"<p>Passwort (klartext): {password}</p>")
print(f"<p>Passwort (SHA-256): {hashed_password}</p>")
