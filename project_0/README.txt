Test files are under: 'source/testfile0'
Sample submissions are under: submission/shell.c'
Autograding result is under: 'results/results.json'

submission_metadata.json is created automatically by gradescope. The functions 'should_rate_limit()' and 'exceed_latency()' are based on submission_metadata.json.

Steps to test locally:
	run 'python run_autograder' and the result will be output to result/results.json
	You can also print the outputs, and set them as visible or invisible: 
	output['stdout_visibility'] = 'visible'

Steps to run on gradescope:
	1. run 'zip -r autograder.zip run_autograder setup.sh  Makefile source/\*'
	2. click on 'Create Programming Assignment' on gradescope
	3. upload autograder.zip to gradescope
	4. upload sample submissions to test


