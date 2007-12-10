import sys
import UBO

#filename="/home/javier/research/RCPSP/problems/testset_ubo10/psp2.sch"
filename = sys.argv[1]
print "filename is:"+filename        
p = UBO.Problem()   
p.readFromFile(filename)
#print p.toString()    
print p.toNddl()    
        
         