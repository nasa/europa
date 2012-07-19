#/usr/bin/python
import sys
import getopt
import UBO

#example :
#export TEST_DIR=$PLASMA_HOME/../benchmarks/UBO/data  
#runTests.py -t10 $TEST_DIR benchmarks_ubo10.txt benchmarks_ubo20.txt
# options -t timeout in seconds
timeoutSecs = 20 

solvers = ['BuiltIn','IFIR','Hybrid']
profiles = {'BuiltIn':'IncrementalFlowProfile', 'IFIR':'TimetableProfile', 'Hybrid':'TimetableProfile' }

opts, args = getopt.getopt(sys.argv[1:], 't:s:')
for o, a in opts:
        if o == "-t":            
            timeoutSecs = int(a)
        if o == "-s":            
            solvers = a.split(',')
            
#print 'timeout is:'+str(timeoutSecs)             
#print 'solvers:'+str(solvers)           

test_dir = args[0]
for file in args[1:]:
    tr = UBO.TestRunner(test_dir,file)
    tr.runTests(timeoutSecs,solvers,profiles)

