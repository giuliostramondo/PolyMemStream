import time
from prf_utils import parseATrace, solveEuristically_getParallelAccesses, Shape

def write_log(logFile,string):
    log = open(logFile, "a")
    log.write(string)
    log.close()

def write_sol(solFile,sol):
    f=open(solFile,"w")
    for s in sol:
        f.write(str(s)+"\n")
    f.close()

atrace_string = parseATrace("europar_polymem_30percent.atrace")

RoCo=[Shape.ROW, Shape.RECTANGLE,Shape.COLUMN]

start_time=time.time()
sol=solveEuristically_getParallelAccesses(atrace_string[0],2,4,RoCo)
time_taken=time.time()-start_time

print "Total number of points:"+str(len(atrace_string[0]))
print "Total number of parallel accesses:"+str(len(sol))
#print "Total duplicates: "+str(len(set(sol))-len(sol))
print "Time taken: "+str(time_taken/60)+" minutes"
#write_log("europar_polymem_30percent_solution.log",str(sol)+"\n");
write_sol("europar_polymem_30percent_solution_RoCo.schedule_v2",sol);
