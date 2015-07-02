#!/usr/bin/env python
import thread # To handle service threads like monitoring
import time # To handle sleep times
import traceback # To pretty-print tracebacks
import os
    
from taskinit import casalog

# Import MPIEnvironment static class
from MPIEnvironment import MPIEnvironment

# Import MPICommunicator singleton
from MPICommunicator import MPICommunicator


class MPIMonitorServer: 
    
    # Storage for the MPIMonitorServer singleton instance reference
    __instance = None
    
    def __init__(self,start_services=True):
        """ Create MPIMonitorServer singleton instance """
        
        casalog_call_origin = "MPIMonitorServer::__init__"
        
        # Check if MPI is effectively enabled
        if not MPIEnvironment.is_mpi_enabled:
            msg = "MPI is not enabled"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check if MPIMonitorServer can be instantiated
        if MPIEnvironment.is_mpi_client:
            msg = "MPIMonitorServer cannot be instantiated at master MPI process"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg  
        
        # Check whether we already have a MPIMonitorServer singleton instance
        if MPIMonitorServer.__instance is None:
            # Create MPIMonitorServer singleton instance
            MPIMonitorServer.__instance = MPIMonitorServer.__MPIMonitorServerImpl(start_services=start_services)

        # Store MPIMonitorServer singleton instance reference as the only member in the handle
        self.__dict__['_MPIMonitorServer__instance'] = MPIMonitorServer.__instance


    def __getattr__(self, attr):
        """ Delegate access to MPIMonitorServer implementation """
        return getattr(self.__instance, attr)


    def __setattr__(self, attr, value):
        """ Delegate access to MPIMonitorServer implementation """
        return setattr(self.__instance, attr, value)        
    
    
    class __MPIMonitorServerImpl:
        """ Implementation of the MPIMonitorServer singleton interface """    
    
        def __init__(self,start_services=True):
            
            # Initialize status state dict
            self.__status = {}
            self.__status['rank'] = MPIEnvironment.mpi_processor_rank
            self.__status['processor'] = MPIEnvironment.hostname
            self.__status['pid'] = os.getpid()
            self.__status['busy'] = False
            self.__status['command'] = None
            self.__status['command_start_time'] = None
            self.__status['command_stop_time'] = None
        
            # Initialize ping status request handler service state
            self.__ping_status_request_handler_service_on = False
            self.__ping_status_request_handler_service_final_round = False
            self.__ping_status_request_handler_service_running = False
            self.__ping_status_request_handler_service_thread = None
            self.__last_ping_status_request_time = None
            self.__client_timeout = False
            
            # Instantiate MPICommunicator reference
            self.__communicator = MPICommunicator()
            
            # Automatically start services
            if start_services:
                self.start_services()
        
        ################################################################################################################            
        # Private methods ##############################################################################################
        ################################################################################################################
        
        
        def __ping_status_request_handler_service(self):
            
            casalog_call_origin = "MPIMonitorServer::ping_status_request_handler_service"
            
            # Mark service as running
            self.__ping_status_request_handler_service_running = True            
        
            while (self.__ping_status_request_handler_service_on or self.__ping_status_request_handler_service_final_round):
                
                # First check if there is a msg available
                msg_available = False
                try:
                    msg_available = self.__communicator.ping_status_request_probe()
                except Exception as instance:
                    casalog.post("Exception checking if ping status request msg is available: %s" 
                                 % str(instance),"SEVERE",casalog_call_origin)
                    msg_available = False
                    
                # Then receive ping status request msg
                msg_received = False
                if (msg_available):
                    self.__last_ping_status_request_time = time.time()
                    try:
                        self.__communicator.ping_status_request_recv()
                        msg_received = True
                    except Exception as instance:
                        
                        casalog.post("Exception receiving ping status request msg: %s" 
                                     % str(instance),"SEVERE",casalog_call_origin)
                        msg_received = False
                
                # jagonzal: Intensive activity in the client can cause monitoring client service to be slowed down
                #           This is due to Python's GIL which is acquired by the CASA SWIG components
                #           Using SWIG's thread option it is possible to disable GIL within the SWIG components
                #           (see test_mpi4casa[test1_applycal_fluxscale_gcal_bcal])
                # Check when we last received a ping status request
                # elif self.__last_ping_status_request_time is not None:
                #    elapsed_time = time.time() - self.__last_ping_status_request_time
                #    if (elapsed_time > MPIEnvironment.mpi_ping_status_request_handler_service_timeout):
                #        casalog.post("Heartbeat from client not received in the last %ss" % 
                #                     str(int(round(elapsed_time))),"WARN",casalog_call_origin)
                #        self.__client_timeout = True
                        
                # Send back response
                if (msg_received):
                    try:
                        self.__communicator.ping_status_response_send(response=self.__status)
                    except:
                        formatted_traceback = traceback.format_exc()
                        casalog.post("Exception sending back ping status response: %s" 
                                     % str(formatted_traceback),"SEVERE",casalog_call_origin)
                else:
                    time.sleep(MPIEnvironment.mpi_ping_status_request_handler_service_sleep_time)
                    
                # Check if this was last round
                if (self.__ping_status_request_handler_service_final_round):
                    self.__ping_status_request_handler_service_final_round = False
            
            # Mark service as not running
            self.__ping_status_request_handler_service_running = False
        
        
        def __start_ping_status_request_handler_service(self):
        
            casalog_call_origin = "MPIMonitorServer::start_ping_status_request_handler_service"     
        
            if self.__ping_status_request_handler_service_running:
                casalog.post("MPI ping status request handler service is already running","WARN",casalog_call_origin)
                return True
            
            try:
                self.__ping_status_request_handler_service_on = True
                self.__ping_status_request_handler_service_thread = thread.start_new_thread(self.__ping_status_request_handler_service, ())
            except Exception as instance:
                self.__ping_status_request_handler_service_on = False
                self.__ping_status_request_handler_service_running = False
                casalog.post("Exception starting MPI ping status request handler service: %s" 
                             % str(instance),"SEVERE",casalog_call_origin)
                return False 
        
            while (not self.__ping_status_request_handler_service_running):
                time.sleep(MPIEnvironment.mpi_check_start_service_sleep_time)
        
            casalog.post("MPI ping status request handler service started","INFO",casalog_call_origin)
            
            return True
        
        
        def __stop_ping_status_request_handler_service(self):
        
            casalog_call_origin = "MPIMonitorServer::stop_ping_status_request_handler_service"
        
            if not self.__ping_status_request_handler_service_on:
                casalog.post("MPI ping status request handler service is not running","WARN",casalog_call_origin)
                return        
        
            self.__ping_status_request_handler_service_final_round = True
            self.__ping_status_request_handler_service_on = False
        
            while (self.__ping_status_request_handler_service_running):
                time.sleep(MPIEnvironment.mpi_check_stop_service_sleep_time)
                
            casalog.post("MPI ping status request handler service stopped","INFO",casalog_call_origin)
            
            
        ################################################################################################################            
        # Public methods ###############################################################################################
        ################################################################################################################            
            
        def start_services(self):
        
            self.__start_ping_status_request_handler_service()
            
            
        def stop_services(self):
        
            self.__stop_ping_status_request_handler_service()   
            
            
        def get_client_timeout(self):
            
            return self.__client_timeout
            
            
        def get_status(self,keyword=None):
            
            casalog_call_origin = "MPIMonitorServer::get_status"
            
            # If no keyword is provided return a copy of the status dictionary
            if keyword is None:
                return dict(self.__status)
            # If keyword is provided check existence and return the mapped value
            elif self.__status.has_key(keyword):
                return self.__status[keyword]
            else:
                casalog.post("Status keyword %s not defined" % str(keyword),"WARN",casalog_call_origin)
            
            
        def set_status(self,keyword,value):
            
            casalog_call_origin = "MPIMonitorServer::set_status"

            if self.__status.has_key(keyword):
                self.__status[keyword] = value
            else:
                casalog.post("Status keyword %s not defined" % str(keyword),"WARN",casalog_call_origin)          

# EOF
