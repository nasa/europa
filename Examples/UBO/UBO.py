
import os
import sys
import StringIO

class Successor:
    def __init__(self,id,d):
        self.actId = id
        self.distance = d
    
    def toString(self):
        buf = StringIO.StringIO()
        print >>buf, '{%d,%d}' % (self.actId,self.distance), 
        return buf.getvalue()
    
class Activity:
    def __init__(self):
        self.id = -1
        self.duration = 0
        self.resUsages = []
        self.succs = []
    
    def toString(self):
        buf = StringIO.StringIO()
        
        print >>buf,'Activity(',self.id,self.duration,
        print >>buf,' Successors:',
        for succ in self.succs:
            buf.write(succ.toString())
            
        print >>buf, ' ResUsage:{',
        for i in xrange(len(self.resUsages)):
            print >>buf, '%d,' % (self.resUsages[i],),
        print >>buf,'}',
               
        print >>buf,')',
        return buf.getvalue()
    
class Resource:
    def __init__(self):
        self.id = -1
        self.capacity = 0
    
    def toString(self):
        buf = StringIO.StringIO()
        print >>buf,'Resource(',self.id,',',self.capacity,')',
        return buf.getvalue()
    
class Problem:
    def __init__(self):
        self.resources = [] 
        self.activities = []
    
    def readActivity(self,line):
        words = line.split()
        actId = int(words[0])
        succCnt = int(words[2])
        
        act = Activity()
        act.id = actId
        for j in xrange(succCnt):
            succ = Successor(int(words[3+j]),int(words[3+j+succCnt].lstrip('[').rstrip(']')))
            act.succs.append(succ)            
        return act                

    def readResourceUsage(self,line,resourceCnt):
        words = line.split()
        actIdx = int(words[0])
        act = self.activities[actIdx]
        act.duration = max(int(words[2]),1) # can't deal with 0 duration
        for j in xrange(resourceCnt):
            act.resUsages.append(int(words[j+3]))

    def readFromFile(self,filename):
        self.filename = filename
        file = open(filename)
        line = file.readline()
        words = line.split()
        actCnt = int(words[0])
        resourceCnt = int(words[1])
            
        for i in xrange(actCnt+2):
            line = file.readline()
            self.activities.append(self.readActivity(line))            

        for i in xrange(actCnt+2):
           line = file.readline()
           self.readResourceUsage(line,resourceCnt)

        line = file.readline()
        words = line.split()
        for i in xrange(resourceCnt):
            r = Resource()
            self.resources.append(r)
            r.id = i
            r.capacity = int(words[i])
        
    def toString(self):
        buf = StringIO.StringIO()
        print >>buf, 'Problem'        
        for r in self.resources:
            print >>buf,'    ',r.toString()  
        for a in self.activities:
            print >>buf,'    ',a.toString()  
        return buf.getvalue()
    
    def toNddl(self):
        buf = StringIO.StringIO()
        
        print >>buf,'// Original file:'+self.filename
        print >>buf,'''
#include "UBO-model.nddl"

PlannerConfig c = new PlannerConfig(0, 1000, +inf, +inf );

ProblemInstance problem = new ProblemInstance();

int maxDuration;
int maxDurationPlusOne;

maxDuration.specify(1000);
addEq(maxDuration,1,maxDurationPlusOne);
'''        
        for r in self.resources:
            print >>buf,'CapacityResource resource%d = new CapacityResource( 0.0 , %d.0 );' % (r.id,r.capacity)  
        
        i=0
        for act in self.activities:
            for j in xrange(len(act.resUsages)):
                if (act.resUsages[j] > 0):
                    # TODO: generate at least one allocation per activity?
                    print >>buf,'Allocation a%d = new Allocation( resource%d , %d , %d.0 );' % (i,j,act.id,act.resUsages[j])
                    i+=1
        print >>buf,'Allocation a%d = new Allocation( resource%d , %d , %d.0 );' % (i,0,self.activities[0].id,0) ; i+=1
        print >>buf,'Allocation a%d = new Allocation( resource%d , %d , %d.0 );' % (i,0,self.activities[len(self.activities)-1].id,0) ; i+=1
        print >>buf,''
            
        for act in self.activities:
            print >>buf,'ActivityTimeline at%d = new ActivityTimeline(%d);' % (act.id,act.id)
        print >>buf,''
                
        print >>buf,'close();'
        print >>buf,''
                
        for act in self.activities:
            print >>buf,'goal( problem.Activity activity%d );' % (act.id,)
            print >>buf,'eq( activity%d.duration, %d );' % (act.id,act.duration)
            print >>buf,'eq( activity%d.m_identifier, %d);' % (act.id,act.id)
            print >>buf,'eq( activity%d.timeline, at%d);' % (act.id,act.id)
            print >>buf,''
                
        for act in self.activities:
            for succ in act.succs:
                if (succ.distance >= 0):
                    predId = act.id
                    succId = succ.actId
                    lb = str(succ.distance)
                    ub = '+inf'
                else:
                    predId = succ.actId
                    succId = act.id
                    lb = '-inf'
                    ub = str(-succ.distance)    
                print >>buf,'temporalDistance( activity%d.start, [ %s %s ], activity%d.start );' % (predId,lb,ub,succId)
        print >>buf,''
                
                
        for act in self.activities:
            print >>buf,'precedes(0,activity%d.start);precedes(activity%d.end,maxDuration);' % (act.id,act.id)
        print >>buf,''
                    
        for act in self.activities:
            print >>buf,'activity%d.activate();' % (act.id,)
                    
        return buf.getvalue()
        

class TestRunner:
    def __init__(self,test_dir,benchmark_file):
        self.test_dir = test_dir
        self.benchmark_file = benchmark_file
        self.problems = [] 
         
    def readProblems(self):
        file = open(self.test_dir+'/'+self.benchmark_file)
        for line in file:
            if (line.startswith('UBO')):
               words = line.split()
               test_file = (words[0].split('-'))[1]
               test_file = 'psp'+test_file.lstrip('0')+'.sch'
               bound = words[1]
               if (bound != 'inf'):
                   self.problems.append((test_file,bound))
         
    def runTests(self,timeoutSecs):
        self.readProblems()
        solvers = ['BuiltIn','IFIR']
        data_dir = self.test_dir+'/testset'+self.benchmark_file.lstrip('benchmarks').rstrip('.txt')
        for p in self.problems:
            problem = Problem()
            problem.readFromFile(data_dir+'/'+p[0])
            nddl = problem.toNddl()
            out = open('UBO-gen-initial-state.nddl','w')
            print >>out,nddl
            out.close()
            for s in solvers:
                cmd = 'ant'+\
                    ' -Dproject.mode=o'+\
                    ' -Dproject.test='+p[0]+\
                    ' -Dproject.bound='+str(int(p[1])+2)+\
                    ' -Dproject.timeout='+str(timeoutSecs)+\
                    ' -Dproject.solver='+s
                print cmd 
                os.system(cmd)
                                    
         