import sys
import re
import json

ANSWERS_FILE='source/expected.json'
SUBMISSION_FILE='submission/shell.c'
RESULTS_FILE='results/results.json'
METADATA_FILE='submission_metadata.json'

with open(ANSWERS_FILE, 'r') as f:
    expected = json.load(f)

with open(METADATA_FILE) as f:
    metadata = json.load(f)