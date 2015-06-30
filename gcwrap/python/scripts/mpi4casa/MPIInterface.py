#!/usr/bin/env python

from taskinit import casalog # Import casalog

import time # To calculate elapsed times

# Import MPIEnvironment static class
from MPIEnvironment import MPIEnvironment

# Import MPICommandClient static class
from MPICommandClient import MPICommandClient

# Import MPIMonitorClient singleton
from MPIMonitorClient import MPIMonitorClient



class MPIInterfaceCore:
    
    # Storage for the MPIInterfaceCore singleton instance reference
    __instance = None


    def __init__(self):
        """ Create MPIInterfaceCore singleton instance """
        
        casalog_call_origin = "MPIInterfaceCore::__init__"
               
        # Check if MPI is effectively enabled
        if not MPIEnvironment.is_mpi_enabled:
            msg = "MPI is not enabled"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check if MPIInterfaceCore can be instantiated
        if not MPIEnvironment.is_mpi_client:
            msg = "MPIInterfaceCore can only be instantiated at master MPI process"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check whether we already have a MPIInterfaceCore singleton instance
        if MPIInterfaceCore.__instance is None:
            # Create MPIInterfaceCore singleton instance
            MPIInterfaceCore.__instance = MPIInterfaceCore.__MPIInterfaceCoreImpl()

        # Store MPIInterfaceCore singleton instance reference as the only member in the handle
        self.__dict__['_MPIInterfaceCore__instance'] = MPIInterfaceCore.__instance
        
        
    def __getattr__(self, attr):
        """ Delegate access to MPIInterfaceCore implementation """
        return getattr(self.__instance, attr)


    def __setattr__(self, attr, value):
        """ Delegate access to MPIInterfaceCore implementation """
        return setattr(self.__instance, attr, value)    
    
    
    
    class __MPIInterfaceCoreImpl:
        """ Implementation of the MPIInterfaceCore singleton interface """

        
        def __init__(self):
            
            self.__command_client = MPICommandClient()
            self.__monitor_client = MPIMonitorClient()
        
        
        def start_cluster(self, cl_file=None):
            
            self.__command_client.set_log_mode(MPIInterface._log_mode)
            self.__command_client.start_services()
            
            
        def stop_cluster(self):

            self.__command_client.stop_services()        
            
            
        def get_engines(self):
            
            return MPIEnvironment.mpi_server_rank_list()
        
        
        def get_nodes(self):
            
            # Get list of all server Ids
            servers_list = MPIEnvironment.mpi_server_rank_list()
            
            # Get list of hostnames for each server Id
            hostnames = []
            for server in servers_list:
                hostnames.append(self.__monitor_client.get_server_status_keyword(server,'processor'))
                
            # Remove duplicates to get the list of host used in general
            hostnames_not_repeated = list(set(hostnames))
            
            return hostnames_not_repeated
        
        
        def pgc(self,commands,block=True):
            """This method has two modes:
            
               - When the input command is a dictionary of commands execute  
                 each command taking the dictionary key as target node
                 (Equivalent to various calls to odo/execute)  
                 
               - When commands is a single command execute it in all engines          
            """
            
            # Get list of jobs and commands
            ret = None
            if isinstance(commands,dict):
                
                # Spawn jobs in non-blocking mode
                jobId_list = []
                for server in commands:
                    cmd = commands[server]
                    jobId = self.__command_client.push_command_request(   cmd,
                                                                          block=False,
                                                                          target_server=server)
                    jobId_list.append(jobId[0])
                
                # If user requests blocking mode wait until execution is completed    
                ret = self.__command_client.get_command_response(jobId_list,block=block,verbose=True)
                    
                
            else:
                cmd = commands
                # Get list of all servers
                all_servers_list = MPIEnvironment.mpi_server_rank_list()
                # Execute command in all servers          
                ret = self.__command_client.push_command_request(   cmd,
                                                                    block=block,
                                                                    target_server=all_servers_list)
            
            # Return result
            return ret        
        
        
        def odo(self, job, nodes):
            """Execute a job on a subset of engines in non-blocking mode"""             
            
            cmd_ids = self.__command_client.push_command_request(job,block=False,target_server=nodes)
            
            return cmd_ids
        
        
        def execute(self, job, nodes):
            """Execute a job on a subset of engines in blocking mode"""             
            
            result_list = self.__command_client.push_command_request(job,block=True,target_server=nodes)
            
            return result_list      
        
        
        def push(self, variables, targets=None):
            """Set variables in a sub-set of engines"""
            
            # Determine target servers
            target_server = []
            if targets is None or targets == 'all':
                target_server = MPIEnvironment.mpi_server_rank_list()
            else:
                target_server = list(targets)
                
            # Push variables
            ret = self.__command_client.push_command_request(   "push",
                                                                block=True,
                                                                target_server=target_server,
                                                                parameters=dict(variables))
            
            # Return request result for further processing
            return ret
            
        
        def pull(self, varname="", targets=None):
            """Retrieve a variable from a sub-set of engines"""
             
            # Pull variable from all servers
            result_list = []
            if targets is None:
                result_list = self.pgc(varname)
            else:
                result_list = self.execute(varname,nodes=targets)
            
            # Extract result vars and corresponding server Ids
            res = {}
            for result in result_list:
                res[result['server']] = result['ret']
                
            # Return result
            return res
        
        
        def check_job(self, jobId, verbose=True):
            """Check the status of a non-blocking job"""
            
            jobId_list = list(jobId)
            command_response_list = self.__command_client.get_command_response(jobId_list,block=False,verbose=verbose)
            
            # Aggregate exceptions and completed jobIds
            error_msg = ''
            completed_jobs = []
            for command_response in command_response_list:
                if not command_response['successful']:
                    if len(error_msg) > 0: error_msg += "\n"
                    
                    error_msg += "Exception executing command in server %s: %s" % (command_response['server'],
                                                                                  command_response['traceback'])
                else:
                    completed_jobs.append(command_response['id'])
                    
            # Re-throw aggregated exception
            if len(error_msg) > 0:
                casalog.post(error_msg,"SEVERE","MPIInterfaceCore::check_job")    
                raise Exception(error_msg) 
                    
            # Check that all jobs have been completed
            completed = True
            for jobId in jobId_list:
                if jobId not in completed_jobs:
                    completed = False
                    break
            
            # Return completion status
            return completed
            
            
        def get_server_status(self):
            
            return self.__command_client.get_server_status()
        
        
        def get_command_request_list(self):
            
            return self.__command_client.get_command_request_list()      
        

        def get_command_response_list(self):
            
            return self.__command_client.get_command_response_list()               

 

class MPIInterface:
    
    # Storage for the MPIInterface singleton instance reference
    __instance = None
    
    # Log mode is static so that it can be accessed by getCluster
    _log_mode = 'unified'


    def __init__(self):
        """ Create MPIInterface singleton instance """
        
        casalog_call_origin = "MPIInterface::__init__"
               
        # Check if MPI is effectively enabled
        if not MPIEnvironment.is_mpi_enabled:
            msg = "MPI is not enabled"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check if MPIInterface can be instantiated
        if not MPIEnvironment.is_mpi_client:
            msg = "MPIInterface can only be instantiated at master MPI process"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check whether we already have a MPIInterface singleton instance
        if MPIInterface.__instance is None:
            # Create MPIInterface singleton instance
            MPIInterface.__instance = MPIInterface.__MPIInterfaceImpl()

        # Store MPIInterface singleton instance reference as the only member in the handle
        self.__dict__['_MPIInterface__instance'] = MPIInterface.__instance
        
    def __getattr__(self, attr):
        """ Delegate access to MPIInterface implementation """
        return getattr(self.__instance, attr)


    def __setattr__(self, attr, value):
        """ Delegate access to MPIInterface implementation """
        return setattr(self.__instance, attr, value)    
      
      
    @staticmethod
    def set_log_mode(logmode):
        
        MPIInterface._log_mode = logmode     
                  
    @staticmethod
    def getCluster():
            
        # Get cluster reference
        if MPIInterface.__instance is None:
            cluster = MPIInterface()
        else:
            cluster = MPIInterface.__instance
            if not cluster.isClusterRunning():
                cluster.init_cluster()
        
        # Initialize cluster
        cluster.start_cluster()
        
        return cluster
    
    
    @staticmethod
    def print_table(table):
        
        col_width = [max(len(x) for x in col) for col in zip(*table)]
        for line in table:
            print "| " + " | ".join("{:{}}".format(x, col_width[i]) for i, x in enumerate(line)) + " |"       
    
    
    
    class __MPIInterfaceImpl:
        """ Implementation of the MPIInterface singleton interface """

        
        def __init__(self):
            
            # Reference to inner cluster object (equivalent to parallel_go)
            self._cluster = MPIInterfaceCore()
            
            # Direct reference to MPICommandClient for methods not resorting to the inner cluster
            self.__command_client = MPICommandClient()
            
            
        def isClusterRunning(self):
            
            if self.__command_client.get_lifecyle_state() == 1:
                return True
            else:
                return False    

            
        def init_cluster(self, clusterfile=None, project=None):
            
            # NOTE: In the MPI framework the clusterfile is processed by mpirun
            #       So it is not necessary to process and validate clusterfile here
            self.start_cluster()
            
            
        def start_cluster(self):
            
            # TODO: This should set OMP_NUM_THREADS as well
            self._cluster.start_cluster()
            
            
        def stop_cluster(self):

            self._cluster.stop_cluster()    
            
    
        def do_and_record(self, cmd, id=None, group='', subMS=''):
            
            jobId = self._cluster.odo(cmd,id)
            return jobId
            
            
        def get_engine_store(self, id):
             
            res = self._cluster.execute('os.getcwd()',id)
             
            return res[0]['ret']
         
         
        def get_status(self):
            
            command_request_list = self._cluster.get_command_request_list()
            command_response_list = self._cluster.get_command_response_list()
            server_status = self._cluster.get_server_status()
            
            # Show first jobs running, then the ones holding the queue and finally the ones already processed
            status_list = ['request sent','holding queue','timeout','response received']
            
            # Generate job status table
            job_status_list = [['JOB ID','SERVER','HOSTNAME','QUEUE STATUS','COMMAND','ELAPSED TIME (s)','EXECUTION STATUS']]
            for status in status_list:
                for jobId in command_request_list:
                        
                    # Generate job status list
                    if command_request_list[jobId]['status'] == status:
                        
                        # Get server to access info from the server status
                        server = command_request_list[jobId]['server']
                        hostname = server_status[server]['processor']
                        
                        # Create job status info
                        job_status = []
                        job_status.append(str(jobId))
                        job_status.append(str(server))
                        job_status.append(hostname)
                        job_status.append(command_request_list[jobId]['status'])
                        job_status.append(command_request_list[jobId]['command'])
                                               
                        # Add run time 
                        elapsed_time = ''
                        if status == status_list[1]: # holding queue
                            elapsed_time = 'N/A'
                        elif status == status_list[0] or status == status_list[2]: # request sent / timeout
                            start_time = server_status[server]['command_start_time'] 
                            if start_time is not None:
                                elapsed_time = "%.2f" % (time.time() - start_time)
                            else:
                                elapsed_time = 'unset'
                        elif status == status_list[3]: # 'response received'
                            start_time = command_response_list[jobId]['command_start_time']
                            stop_time = command_response_list[jobId]['command_stop_time']
                            if start_time is not None and stop_time is not None:
                                elapsed_time = "%.2f" % (stop_time- start_time)
                            else:
                                elapsed_time = 'unset'
                                                       
                        job_status.append(elapsed_time)
                        
                        # Add job execution status
                        execution_status = ''
                        if status == status_list[0] or status == status_list[1]: # request sent / holding queue
                            execution_status = 'pending'
                        elif status == status_list[2]: # timeout
                            execution_status = 'timeout'
                        elif status == status_list[3]: # response received
                            if command_response_list[jobId]['successful']:
                                ret = command_response_list[jobId]['ret']
                                if isinstance(ret,bool):
                                    if ret == True:
                                        execution_status = 'completed - True'
                                    else:
                                        execution_status = 'completed - False'
                                else:
                                    execution_status = 'completed'
                            else:
                                execution_status = 'exception raised'
                                
                        job_status.append(execution_status)
                        
                        # Append job status info to list
                        job_status_list.append(job_status)
                        
            # Print job status table
            MPIInterface.print_table(job_status_list)
            
            # Return job status table
            return job_status_list     
        
        
        def set_log_level(self,log_level):
            
            self.__command_client.set_log_level(log_level)
             
            
# EOF
