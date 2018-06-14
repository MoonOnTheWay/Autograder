Test files are under: 'source/'
Sample submissions are under: submission/' (You can also get more submissions from submit.cs.ucsb.edu)
Autograding result is under: 'results/results.json'

submission_metadata.json is created automatically by gradescope. The functions 'should_rate_limit()' and 'exceed_latency()' are based on submission_metadata.json.

Steps to test locally:
	run 'python run_autograder' and the result will be output to result/results.json
	You can also print the outputs, and set them as visible or invisible: 
	output['stdout_visibility'] = 'visible'

Steps to run on gradescope:
	1. run 'cp -r source/ .'
	2. run 'zip -r autograder.zip run_autograder setup.sh tests.yaml harness_code compile.sh'
	3. click on 'Create Programming Assignment' on gradescope
	4. upload autograder.zip to gradescope
	5. upload sample submissions to test


