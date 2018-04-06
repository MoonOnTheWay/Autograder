import sys
import re
import json
import os
import subprocess

ANSWERS_FILE='source/testfile'
RESULTS_FILE='results/results.json'
METADATA_FILE='submission_metadata.json'

def main():
	subprocess.call(["cp", "-r", "submission/", "."])
	subprocess.call(["make"])

	output = {}
	tests = []
	for filename in os.listdir('.'):
		if not filename.startswith('testfile'):
			continue
		returncode = subprocess.call(["./shell < " + filename], shell = True)
		if returncode == 0:
			score = 1
			result = "Passed"
		else:
			score = 0
			result = "Not Passed"
		tests.append({'score': score, "output": result})
	output['tests'] = tests

	with open(RESULTS_FILE, 'w') as f:
		f.write(json.dumps(output))

main()