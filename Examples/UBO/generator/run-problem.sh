
#!/bin/bash

problem_name=$1
bound=$2
generator_dir=generator
problems_dir=~/research/RCPSP/problems/testset_ubo10

$generator_dir/generator --problem  $problems_dir/$problem_name.sch $problem_name $bound
./nddl2xml $problem_name.$bound.nddl ../PLASMA/NDDL/base/jars/nddl.jar
./UBO-planner_o_rt $problem_name.$bound.xml PlannerConfig.xml
