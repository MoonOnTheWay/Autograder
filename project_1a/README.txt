Test files are under: 'source/'
Sample submissions are under: submission/' (You can also get more submissions from submit.cs.ucsb.edu)
Autograding result is under: 'results/results.json'

submission_metadata.json is created automatically by gradescope. The functions 'should_rate_limit()' and 'exceed_latency()' are based on submission_metadata.json.

Steps to test locally:
	run 'python run_autograder' and the result will be output to result/results.json
	You can also print the outputs, and set them as visible or invisible: 
	output['stdout_visibility'] = 'visible'

Steps to run on gradescope:
	1. run 'zip -r autograder.zip . -x submission/\* -x source/\* -x hashchain.* -x rwlock.* -x submission_metadata.json'
	2. click on 'Create Programming Assignment' on gradescope
	3. upload autograder.zip to gradescope
	4. upload sample submissions to test


