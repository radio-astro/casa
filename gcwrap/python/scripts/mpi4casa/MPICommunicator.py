#!/usr/bin/env python
import atexit # To handle destructors

# Import MPIEnvironment static class
from MPIEnvironment import MPIEnvironment


class MPICommunicator: 
    
    # Storage for the MPICommunicator singleton instance reference
    __instance = None
    
    def __init__(self):
        """ Create MPICommunicator singleton instance """
                       
        # Check if MPI is effectively enabled
        if not MPIEnvironment.is_mpi_enabled:
            msg = "MPI is not enabled"
            raise Exception,msg
        
        # Check whether we already have a MPIClient singleton instance
        if MPICommunicator.__instance is None:
            # Create MPIClient singleton instance
            MPICommunicator.__instance = MPICommunicator.__MPICommunicatorImpl()

        # Store MPIClient singleton instance reference as the only member in the handle
        self.__dict__['_MPICommunicator__instance'] = MPICommunicator.__instance


    def __getattr__(self, attr):
        """ Delegate access to MPICommunicator implementation """
        return getattr(self.__instance, attr)


    def __setattr__(self, attr, value):
        """ Delegate access to MPICommunicator implementation """
        return setattr(self.__instance, attr, value)        
    
    
    class __MPICommunicatorImpl:
        """ Implementation of the MPICommunicator singleton interface """    
    
        def __init__(self):
            
            # Initialize communicators
            try:
                self.__command_channel = 2
                self.__ping_status_channel = 1
                self.__control_service_channel = 0
                self.__command_request_communicator = MPIEnvironment.mpi_comm_world_factory()
                self.__ping_status_request_communicator = MPIEnvironment.mpi_comm_world_factory()
                self.__command_response_communicator = MPIEnvironment.mpi_comm_world_factory()
                self.__ping_status_response_communicator = MPIEnvironment.mpi_comm_world_factory()
                self.__control_service_request_communicator = MPIEnvironment.mpi_comm_world_factory()
                self.__control_service_response_communicator = MPIEnvironment.mpi_comm_world_factory()
                
                # Register exit handler in case MPICommandClient exit handler is not initialized
                # NOTE: It is not guaranteed that __del__() methods are called 
                #       for objects that still exist when the interpreter exits.
                self.__servers_running = True
                # jagonzal: This exit function must be registered only for the client
                if MPIEnvironment.is_mpi_client:                   
                    atexit.register(self.finalize_server_and_client_mpi_environment)
            
            except Exception, instance:
                self.__command_channel = -1
                self.__ping_status_channel = -1
                self.__control_service_channel = -1               
                self.__command_request_communicator = None
                self.__ping_status_request_communicator = None
                self.__command_response_communicator = None
                self.__ping_status_response_communicator = None
                self.__control_service_request_communicator = None
                self.__control_service_response_communicator = None
                msg = "Exception initializing MPICommunicator at processor with rank "
                msg = msg + "%s: %s" % (str(MPIEnvironment.mpi_processor_rank),str(instance))
                raise Exception,msg
            
        def finalize_server_and_client_mpi_environment(self):
            
            # Send stop server signal to servers
            self.control_service_request_broadcast(request={'signal':'exit'})
            
            # Finalize local MPI envionment
            MPIEnvironment.finalize_mpi_environment()
            
                    
        def command_request_send(self,request,server):
            self.__command_request_communicator.bsend(obj=request,
                                                      dest=server,
                                                      tag=self.__command_channel)   
        
        
        def command_request_probe(self):
            return self.__command_request_communicator.Iprobe(source=MPIEnvironment.mpi_client_rank,
                                                              tag=self.__command_channel)
        
        
        def command_request_recv(self):
            return self.__command_request_communicator.recv(source=MPIEnvironment.mpi_client_rank,
                                                            tag=self.__command_channel)
        
        
        def command_response_send(self,response):
            self.__command_response_communicator.bsend(obj=response,
                                                       dest=MPIEnvironment.mpi_client_rank,
                                                       tag=self.__command_channel)
            
            
        def command_response_probe(self):
            return self.__command_response_communicator.Iprobe(source=MPIEnvironment.mpi_any_source,
                                                               tag=self.__command_channel)            
        
        
        def command_response_recv(self):
            return self.__command_response_communicator.recv(source=MPIEnvironment.mpi_any_source,
                                                             tag=self.__command_channel)
        
        
        def ping_status_request_send(self,server):
            self.__ping_status_request_communicator.bsend(obj={},
                                                          dest=server,
                                                          tag=self.__ping_status_channel)         
             
             
        def ping_status_request_probe(self):
            return self.__ping_status_request_communicator.Iprobe(source=MPIEnvironment.mpi_client_rank,
                                                                  tag=self.__ping_status_channel)          
        
        
        def ping_status_request_recv(self):
            return self.__ping_status_request_communicator.recv(source=MPIEnvironment.mpi_client_rank,
                                                                tag=self.__ping_status_channel)       
        
        
        def ping_status_response_send(self,response):
            self.__ping_status_response_communicator.bsend(obj=response,
                                                           dest=MPIEnvironment.mpi_client_rank,
                                                           tag=self.__ping_status_channel)
            
            
        def ping_status_response_probe(self):
            return self.__ping_status_response_communicator.Iprobe(source=MPIEnvironment.mpi_any_source,
                                                                   tag=self.__ping_status_channel)                        
        
        
        def ping_status_response_recv(self):
            return self.__ping_status_response_communicator.recv(source=MPIEnvironment.mpi_any_source,
                                                                 tag=self.__ping_status_channel)
        
        
        def control_service_request_send(self,request,server):
            self.__control_service_request_communicator.bsend(obj=request,
                                                              dest=server,
                                                              tag=self.__control_service_channel)
            
            
        def control_service_request_probe(self):
            return self.__control_service_request_communicator.Iprobe(source=MPIEnvironment.mpi_client_rank,
                                                                      tag=self.__control_service_channel)
        
        
        def control_service_request_recv(self):
            return self.__control_service_request_communicator.recv(source=MPIEnvironment.mpi_client_rank,
                                                                    tag=self.__control_service_channel)    
        
        
        def control_service_response_send(self,response):
            self.__control_service_response_communicator.bsend(obj=response,
                                                               dest=MPIEnvironment.mpi_client_rank,
                                                               tag=self.__control_service_channel)
            
            
        def control_service_response_probe(self):
            return self.__control_service_response_communicator.Iprobe(source=MPIEnvironment.mpi_any_source,
                                                                       tag=self.__control_service_channel)
        
        
        def control_service_response_recv(self):
            return self.__control_service_response_communicator.recv(source=MPIEnvironment.mpi_any_source,
                                                                     tag=self.__control_service_channel)    
        
         
        # Convenience method to send control service request to all servers
        # NOTE: We don't use broadcast because it is always blocking
        def control_service_request_broadcast(self,request,logger=None):
            
            if self.__servers_running:
                mpi_server_rank_list = MPIEnvironment.mpi_server_rank_list()
                for rank in mpi_server_rank_list:
                    if logger is not None:
                        logger.post("Sending %s service signal to server %s" 
                                    % (request['signal'],str(rank)),"INFO","MPICommunicator::control_service_request_send_all")
                    self.control_service_request_send(request=request,server=rank)      
                    
        
        # Set method to notify whether the servers are running or not         
        def set_servers_running(self,servers_running):
            self.__servers_running = servers_running
            
            
# EOF