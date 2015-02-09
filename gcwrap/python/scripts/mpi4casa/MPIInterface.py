#!/usr/bin/env python

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
        
        
        def start_engines(self):
            
            self.__command_client.start_services()
            
            
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
        
        
        def pgc(self,args,kwargs=None):
            """Execute a dictionary of jobs with defined target nodes or the same job in all engines
               (Equivalent to various calls to odo/execute)
            
            @param *args: There are 2 modes for this parameter
                - Simple command string to be executed in all engines            
                - Dictionary of commands where the key of the dictionary is the engine id
                
            @param **kwargs: Dictionary with the job execution options
                - block: Boolean (True/False)
            
            """
            
            # Determine job execution mode
            block_mode=True
            if isinstance(kwargs,dict):
                for key in kwargs:
                    if key.lower()=='block': block_mode = kwargs[key]
                    
            # Get list of jobs and commands
            ret = None
            if isinstance(args,dict):
                
                # Spawn jobs in non-blocking mode
                jobId_list = []
                for server in args:
                    cmd = args[server]
                    jobId = self.__command_client.push_command_request(   cmd,
                                                                          block=False,
                                                                          target_server=server)
                    jobId_list.append(jobId[0])
                
                # If user requests blocking mode wait until execution is completed    
                ret = self.__command_client.get_command_response(jobId_list,block=block_mode,verbose=True)
                    
                
            else:
                cmd = args
                # Get list of all servers
                all_servers_list = MPIEnvironment.mpi_server_rank_list()
                # Execute command in all servers          
                ret = self.__command_client.push_command_request(   cmd,
                                                                    block=block_mode,
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
            
            command_response = self.__command_client.get_command_response(jobId,block=False,verbose=verbose)

            if len(command_response) == len(list(jobId)):
                return True
            else:
                return False
            
            
        def get_server_status(self):
            
            return self.__command_client.get_server_status()
        
        
        def get_command_request_list(self):
            
            return self.__command_client.get_command_request_list()      
        

        def get_command_response_list(self):
            
            return self.__command_client.get_command_response_list()               

 

class MPIInterface:
    
    # Storage for the MPIInterface singleton instance reference
    __instance = None


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
    def getCluster():
            
        # Get cluster reference
        if MPIInterface.__instance is None:
            cluster = MPIInterface()
        else:
            cluster = MPIInterface.__instance
        
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
            
            self._cluster = MPIInterfaceCore()

            
        def init_cluster(self, clusterfile='', project=''):
            
            # NOTE: In the MPI framework the clusterfile is processed by mpirun
            #       So it is not necessary to process and validate clusterfile here
            self.start_cluster()
            
            
        def start_cluster(self):
            
            # TODO: This should set OMP_NUM_THREADS as well
            self._cluster.start_engines()
            
            
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
            job_status_list = [['SERVER','HOSTNAME','STATUS','COMMAND','ELAPSED TIME']]
            for status in status_list:
                for jobId in command_request_list:
                        
                    # Generate job status list
                    if command_request_list[jobId]['status'] == status:
                        
                        # Get server to access info from the server status
                        server = command_request_list[jobId]['server']
                        hostname = server_status[server]['processor']
                        
                        # Create job status info
                        job_status = []
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
                                elapsed_time = str(int(round(time.time() - start_time)))
                            else:
                                elapsed_time = 'unset'
                        elif status == status_list[3]: # 'response received'
                            start_time = command_response_list[jobId]['command_start_time']
                            stop_time = command_response_list[jobId]['command_stop_time']
                            if start_time is not None and stop_time is not None:
                                elapsed_time = str(int(round(stop_time - start_time)))
                            else:
                                elapsed_time = 'unset'
                                                       
                        job_status.append(elapsed_time)
                        
                        # Append job status info to list
                        job_status_list.append(job_status)
                        
            # Print job status table
            MPIInterface.print_table(job_status_list)
            
            # Return job status table
            return job_status_list     
             
            
# EOF
