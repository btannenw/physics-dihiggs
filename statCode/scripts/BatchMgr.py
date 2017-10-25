import os
import sys
import commands
import subprocess
import time
import tarfile
import glob

def create_python_command(conf_file, outversion, algs):
    command = ["python", "scripts/doActions.py"]
    conf_dict = config_to_dict(conf_file)
    for c in algs:
        c = str(c).format(**conf_dict)
        command.append(str(c))
    command.append(conf_file)
    command.append(outversion)
    return command

def config_to_dict(conf_file):
    with open(conf_file) as f:
        return dict(l.split() for l in f if not l.isspace() and len(l.split())==2)

def run_local_batch(configs, outversion, algs):
    logdir = "logs"
    try:
        os.mkdir(logdir)
    except OSError:
        pass
    pids=[]
    logfiles=[]
    NCORES = 6
    args = []
    #if
    for c in configs:
        if len(pids) >= NCORES: # manage number of jobs running
            wait_completion(pids)
        print "Launching job for config", c
        cutv, outv = get_ws_name(c, outversion)
        ws_name = cutv+'.'+outv
        conf_dict = config_to_dict(c)
        ws_name = ws_name.format(**conf_dict)
        exec_sequence = create_python_command(c, outversion, algs)
        #print ws_name
        #print exec_sequence
        logfile = "{0}/output_{1}.log".format(logdir, ws_name)
        #sys.exit()
        res = submit_local_job(exec_sequence, logfile)
        pids.append(res[0])
        logfiles.append(res[1])

    # Now just wait for completion of all jobs
    wait_all(pids)
    for f in logfiles:
        f.close()

def run_lxplus_batch(configs, outversion, algs, queue='8nh', jobs = 1, subJob = -1):
    conf0 = configs[0]
    tarfilename = "stuff.tar.gz"
    cutv, outv = get_ws_name(conf0, outversion)
    # FIXME JWH
    # tar_tag = cutv+'.'+(outv.split('_')[0])
    if not outversion.rfind('_') == -1: tar_tag = cutv+'.'+(outversion[:outversion.rfind('_')])
    else: tar_tag = cutv + '.'+ outversion
    inputdir = os.path.join(os.environ["PWD"],"inputs")
    workdir = os.path.join(os.environ["HOME"].replace("user","work"),"analysis/statistics/batch/",tar_tag)
    os.system("mkdir -vp "+workdir)
    if not tarfilename in os.listdir(workdir):
        print "Making tar file for", cutv
        print workdir
        inputs = ["scripts", "src", "include", "Makefile", "*.C", "setup.sh", "HistFactorySchema.dtd", "macros/*.C","macros/*.h","macros/flavTagSFunfold/*.cpp","macros/flavTagSFunfold/*.env","macros/flavTagSFunfold/*.root","transform", "addData.cxx"]
        for c in configs:
            cutv, outv = get_ws_name(c, outversion)
            ws_name = cutv+'.'+outv.split('_',1)[0]
            inputs.append("workspaces/"+ws_name+"_*")
        tar = tarfile.open(os.path.join(workdir,tarfilename), "w:gz")
        inputs = set(inputs)
        for input in inputs:
            for file in glob.glob(input):
                print "  ", file
                tar.add(file)
        tar.close()

    for c in configs:
        for ijob in range(0, jobs):
            if subJob >= 0 and ijob != subJob:
                continue
            print "Launching job " + str(ijob) + " of " + str(jobs) + " for config", c
            jobStr = ""
            if jobs > 1:
                jobStr = "_job" + str(ijob) + "of" + str(jobs)
            cutv, outv = get_ws_name(c, outversion+jobStr)
            ws_name = cutv+'.'+outv
            print ws_name
        # copy config to batch area
            jobname = outv.split('_',1)[1]
            jobdir = os.path.join(workdir, jobname)
            os.system("mkdir -vp "+jobdir)
            conf = c.split('/')[1]
            cpath = os.path.join(jobdir, conf)
            if not conf in os.listdir(workdir):
                subprocess.check_call(["cp", "-v", c, cpath])
            exec_sequence = create_python_command(cpath, outversion+jobStr, algs)
        # put command into string
            cmd = "'"
            for t in exec_sequence:
                cmd += " "+t
            cmd += "'"
            runfile = "run.sh"
            sedcmd = "sed -e \"s:XXX_MYTAR:" + workdir + ":g\" -e \"s:XXX_INPUTS:" + inputdir + ":g\" -e \"s:XXX_MYAFS:" + jobdir + ":g\" -e \"s:XXX_MYCMD:" + cmd + ":g\" scripts/run_batch.sh > " + runfile
            os.system( sedcmd )
            submit_batch_job(ws_name, runfile, queue)
    print "done"

def wait_all(pids):
    """Wait until completion of all launched jobs"""
    while len(pids)>0:
        wait_completion(pids)
    print "All jobs finished !"

def wait_completion(pids):
    """Wait until completion of one of the launched jobs"""
    while True:
        for pid in pids:
            if pid.poll() is not None:
                print "Process", pid.pid, "has completed"
                pids.remove(pid)
                return
        print "Waiting completion of jobs..."
        time.sleep(30) # wait 15 seconds before retrying

def submit_local_job(exec_sequence, logfilename):
    os.system("rm -f "+logfilename)
    output_f=open(logfilename, 'w')
    pid = subprocess.Popen(exec_sequence, stderr=output_f, stdout = output_f)
    return pid, output_f


def submit_batch_job(jobname, runfile, queue='8nh'):
    subcmd = "bsub -L\"/bin/bash\" -q {0} -J {1} < {2}".format(queue, jobname, runfile)
    os.system( subcmd )
    time.sleep(1) # wait 5 seconds before submitting another job
    os.system( "rm " + runfile )


def get_ws_name(conf, outver):
    suff = "_"+conf.rsplit(".",1)[0].rsplit('/',1)[1]
    #
    outversion = outver+suff
    cutver = commands.getoutput("grep \"InputVersion \" {0} | awk -F ' ' '{{print $2}}'".format(conf)).lstrip(' \t')
    #print '*', suff,'*', cutver,'*',outversion,'*'
    return cutver, outversion

#function for lyon batch

def run_lyon_batch(configs, outversion, algs,mail_address="", queue='long', jobs = 1, subJob = -1):
    conf0 = configs[0] #fichier de config
    tarfilename = "stuff.tar.gz"
    cutv, outv = get_ws_name(conf0, outversion)
    tar_tag = cutv+'.'+(outv.split('_')[0])
    username = os.environ["USER"]
    if os.getenv("WORKDIR") :
        inputdir = os.path.join(os.environ["WORKDIR"],"inputs")
    else :
        inputdir = os.path.join(os.environ["PWD"],"inputs")

    workdir = "/sps/atlas/"+username[0]+"/"+username+"/BatchLyonWorkingDir/"+tar_tag
    os.system("mkdir -vp "+workdir)

    if not tarfilename in os.listdir(workdir):
        print "Making tar file for", cutv
        print workdir
        inputs = ["scripts", "src", "include", "Makefile", "*.C", "setup.sh", "HistFactorySchema.dtd", "macros/*.C","macros/*.h","macros/flavTagSFunfold/*.cpp","macros/flavTagSFunfold/*.env","macros/flavTagSFunfold/*.root","transform", "addData.cxx"]
        for c in configs:
            cutv, outv = get_ws_name(c, outversion)
            ws_name = cutv+'.'+outv.split('_',1)[0]
            inputs.append("workspaces/"+ws_name+"_*")
            #print ws_name
        tar = tarfile.open(os.path.join(workdir,tarfilename), "w:gz")
        inputs = set(inputs)
        for input in inputs:
            for file in glob.glob(input):
                print "  ", file
                tar.add(file)
        tar.close()
    #print configs

    for c in configs:
        for ijob in range(0, jobs):
            if subJob >= 0 and ijob != subJob:
                continue
            print "Launching job " + str(ijob) + " of " + str(jobs) + " for config", c
            jobStr = ""
            if jobs > 1:
                jobStr = "_job" + str(ijob) + "of" + str(jobs)
            cutv, outv = get_ws_name(c, outversion+jobStr)
            ws_name = cutv+'.'+outv
            #print ws_name
            # copy config to batch area
            jobname = outv.split('_',1)[1]
            jobdir = os.path.join(workdir, jobname)
            os.system("mkdir -vp "+jobdir)
            conf = c.split('/')[1]
            cpath = os.path.join(jobdir, conf)
            if not conf in os.listdir(workdir):
                subprocess.check_call(["cp", "-v", c, cpath])
            exec_sequence = create_python_command(cpath, outversion+jobStr, algs)
            # put command into string
            cmd = "'"
            for t in exec_sequence:
                cmd += " "+t
            cmd += "'"
            runfile = "run"+ws_name+".sh"
            sedcmd = "sed -e \"s:XXX_MYTAR:" + workdir + ":g\" -e \"s:XXX_INPUTS:" + inputdir + ":g\" -e \"s:XXX_MYAFS:" + jobdir + ":g\" -e \"s:XXX_MYCMD:" + cmd + ":g\" scripts/run_batch.sh > " + runfile
            os.system( sedcmd )
            submit_lyon_job("Jobs"+ws_name, runfile, queue,mail_address) #because job name can't start with number
    print "done"

def submit_lyon_job(jobname, runfile, queue='long',mail_address=""):

    mail_str = ""
    if not(mail_address == "") :
        mail_str = "-m e -M "+mail_address

    if os.getenv("WORKDIR") :
        path = os.environ["WORKDIR"]+"/BatchLyonLog"
    else :
        path = os.environ["PWD"]+"/BatchLyonLog"

    #    print path
    os.system("mkdir -vp "+path)

    #print path
    path_OU = path +"/"+ jobname + ".OU"
    path_ER = path +"/"+ jobname + ".ER"
    subcmd = "qsub "+mail_str+" -l sps=1 -P P_atlas -q {0} -N {1} -o {3} -e {4} {2}".format(queue, jobname, runfile,path_OU,path_ER)
    #    print subcmd
    os.system( subcmd )
    time.sleep(1) # wait 5 seconds before submitting another job
    os.system( "rm " + runfile )
