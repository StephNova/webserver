#!/usr/bin/env python3
import os
import json
import sys

absolute_upload = os.environ.get('ABSOLUTE_UPLOAD', '')



print("Content-Type: application/json\n")

try:
	files = os.listdir(absolute_upload)
	files = [f for f in files 
	  if os.path.isfile(os.path.join(absolute_upload, f)) and not f.startswith('.')]
	print(json.dumps(files))
except Exception as e:
	print(json.dumps({"error": str(e)}))
