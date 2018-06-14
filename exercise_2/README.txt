Take exercise_1 as an example. 
exercise_2 to exercise_6 follow the same steps.

Expected answer is under: 'source/expected_1.json'
Students' submission is under: submission/exercise_1.txt'
Autograding result is under: 'results/results.json'

submission_metadata.json is created automatically by gradescope. The functions 'should_rate_limit()' and 'exceed_latency()' are based on submission_metadata.json.

Steps to test locally:
	run 'python run_autograder' and the result will be output to result/results.json

Steps to run on gradescope:
	1. copy 'exercise_1/source/expected_1.json' to root directory: 'exercise_1/expected_1.json'
	2. run 'zip autograder.zip run_autograder setup.sh expected_1.json'
	3. click on 'Create Programming Assignment' on gradescope
	4. upload autograder.zip to gradescope
	5. upload sample submissions to test


