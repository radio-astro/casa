#!/usr/bin/env python
import thread # To handle service threads like monitoring
import time # To handle sleep times
import traceback # To pretty-print tracebacks

from taskinit import casalog

# Import MPIEnvironment static class
from MPIEnvironment import MPIEnvironment

# Import MPICommunicator singleton
from MPICommunicator import MPICommunicator
        

class MPIMonitorClient:
    
    
    # Storage for the MPIMonitorClient singleton instance reference
    __instance = None


    def __init__(self,start_services=True):
        """ Create MPIMonitorClient singleton instance """
        
        casalog_call_origin = "MPIMonitorClient::__init__"
               
        # Check if MPI is effectively enabled
        if not MPIEnvironment.is_mpi_enabled:
            msg = "MPI is not enabled"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check if MPIMonitorClient can be instantiated
        if not MPIEnvironment.is_mpi_client:
            msg = "MPIMonitorClient can only be instantiated at master MPI process"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check whether we already have a MPIMonitorClient singleton instance
        if MPIMonitorClient.__instance is None:
            # Create MPIMonitorClient singleton instance
            MPIMonitorClient.__instance = MPIMonitorClient.__MPIMonitorClientImpl(start_services=start_services)

        # Store MPIMonitorClient singleton instance reference as the only member in the handle
        self.__dict__['_MPIMonitorClient__instance'] = MPIMonitorClient.__instance        
            

    def __getattr__(self, attr):
        """ Delegate access to MPIMonitorClient implementation """
        return getattr(self.__instance, attr)


    def __setattr__(self, attr, value):
        """ Delegate access to MPIMonitorClient implementation """
        return setattr(self.__instance, attr, value)    
    
    
    class __MPIMonitorClientImpl:
        """ Implementation of the MPIMonitorClient singleton interface """
        
        
        def __init__(self,start_services=True):
                                 
            # Initialize server status state
            self.__server_status_list = {}
            mpi_server_rank_list = MPIEnvironment.mpi_server_rank_list()
            for rank in mpi_server_rank_list:
                self.__server_status_list[rank] = {}
                self.__server_status_list[rank]['rank'] = rank
                self.__server_status_list[rank]['processor'] = None
                self.__server_status_list[rank]['pid'] = None
                self.__server_status_list[rank]['busy'] = False
                self.__server_status_list[rank]['command'] = None
                self.__server_status_list[rank]['command_start_time'] = None
                self.__server_status_list[rank]['pong_pending'] = False
                self.__server_status_list[rank]['ping_time'] = None
                self.__server_status_list[rank]['pong_time'] = None          
                self.__server_status_list[rank]['timeout'] = False      
                   
            # Initialize monitor service state
            self.__monitor_status_service_on = False
            self.__monitor_status_service_running = False
            self.__monitor_status_service_thread = None          
            
            # Initialize ping status response handler service state
            self.__ping_status_response_handler_service_on = False
            self.__ping_status_response_handler_service_running = False
            self.__ping_status_response_handler_service_thread = None  
            
            # Instantiate MPICommunicator reference
            self.__communicator = MPICommunicator()
            
            # Automatically start services
            if start_services:
                self.start_services()
                
                
        ################################################################################################################            
        # Private methods ##############################################################################################
        ################################################################################################################                   
                
                
        def __monitor_status_service(self):
            
            casalog_call_origin = "MPIMonitorClient::monitor_status_service"
            
            # Mark service as running
            self.__monitor_status_service_running = True            
            
            mpi_server_rank_list = MPIEnvironment.mpi_server_rank_list()
                          
            while (self.__monitor_status_service_on):
                # Iterate over servers     
                for rank in mpi_server_rank_list:
                    # Send ping status request if there is none pending
                    if not self.__server_status_list[rank]['pong_pending']:
                        try:
                            self.__communicator.ping_status_request_send(server=rank)
                            self.__server_status_list[rank]['ping_time'] = time.time()
                            self.__server_status_list[rank]['pong_pending'] = True    
                            self.__server_status_list[rank]['pong_checks'] = 0           
                        except:
                            formatted_traceback = traceback.format_exc()
                            casalog.post("Exception sending ping status request to server %s: %s" % 
                                         (str(rank),str(formatted_traceback)),"SEVERE",casalog_call_origin)
                    else:
                        self.__server_status_list[rank]['pong_checks'] += 1
                        elapsed_time = MPIEnvironment.mpi_monitor_status_service_heartbeat
                        elapsed_time *= self.__server_status_list[rank]['pong_checks']
                        # elapsed_time = int(round(time.time() - self.__server_status_list[rank]['ping_time']))                        
                        # Notify when a server reaches timeout condition
                        if ((elapsed_time > MPIEnvironment.mpi_monitor_status_service_timeout) and 
                            (not self.__server_status_list[rank]['timeout'])):
                            casalog.post("Ping status response from server %s not received in the last %ss" % 
                                         (str(rank),str(int(elapsed_time))),"SEVERE",casalog_call_origin)
                            self.__server_status_list[rank]['timeout'] = True
                # Sleep before next round
                time.sleep(MPIEnvironment.mpi_monitor_status_service_heartbeat)
            
            # Mark service as not running
            self.__monitor_status_service_running = False            

            
        def __start_monitor_status_service(self):
        
            casalog_call_origin = "MPIMonitorClient::start_monitor_status_service"

            if self.__monitor_status_service_running:
                casalog.post("MPI monitor status service is already running","WARN",casalog_call_origin)
                return True
            
            try:
                self.__monitor_status_service_on = True
                self.__monitor_status_service_thread = thread.start_new_thread(self.__monitor_status_service, ())
            except:
                formatted_traceback = traceback.format_exc()
                self.__monitor_status_service_on = False
                self.__monitor_status_service_running = False
                casalog.post("Exception starting MPI monitor status service: %s" 
                             % str(formatted_traceback),"SEVERE",casalog_call_origin)  
                return False
        
            while (not self.__monitor_status_service_running):
                time.sleep(MPIEnvironment.mpi_check_start_service_sleep_time) 
        
            casalog.post("MPI monitor status service started","INFO",casalog_call_origin)
            
            return True
        
        
        def __stop_monitor_status_service(self):
        
            casalog_call_origin = "MPIMonitorClient::stop_monitor_status_service"
        
            if not self.__monitor_status_service_running:
                casalog.post("MPI ping status response handler service is not running","WARN",casalog_call_origin)
                return             

            self.__monitor_status_service_on = False
        
            while (self.__monitor_status_service_running):
                time.sleep(MPIEnvironment.mpi_check_stop_service_sleep_time)
                
            casalog.post("MPI monitor status service stopped","INFO",casalog_call_origin)  
                
                
        def __ping_status_response_handler_service(self):
            
            casalog_call_origin = "MPIMonitorClient::ping_status_response_handler_service"
            
            # Mark service as running
            self.__ping_status_response_handler_service_running = True            
                          
            while (self.__ping_status_response_handler_service_on):
                
                # First check if there is a ping_status response msg available
                msg_available = False
                try:
                    msg_available = self.__communicator.ping_status_response_probe()
                except:
                    msg_available = False
                    formatted_traceback = traceback.format_exc()
                    casalog.post("Exception checking if ping status response msg is available: %s" 
                                 % str(formatted_traceback),"SEVERE",casalog_call_origin)
                    
                # Then receive, store and post ping_status response msg
                if (msg_available):            
                    try:
                        ping_status_response = self.__communicator.ping_status_response_recv()
                        pong_time = time.time()
                        rank = ping_status_response['rank']
                        self.__server_status_list[rank]['command'] = ping_status_response['command']
                        self.__server_status_list[rank]['command_start_time'] = ping_status_response['command_start_time']
                        self.__server_status_list[rank]['pong_time'] = pong_time
                        self.__server_status_list[rank]['pong_pending'] = False
                        elapsed_time = pong_time - self.__server_status_list[rank]['ping_time']
                        # Notify if the response has been received after timeout
                        if self.__server_status_list[rank]['timeout']:
                            self.__server_status_list[rank]['timeout'] = False
                            casalog.post("Ping status response from server %s finally received after %ss" % 
                                         (str(rank),str(int(elapsed_time))),"WARN",casalog_call_origin)                  
                    except:
                        formatted_traceback = traceback.format_exc()
                        casalog.post("Exception receiving ping status response msg: %s" 
                                     % str(formatted_traceback),"SEVERE",casalog_call_origin)
                else:
                    time.sleep(MPIEnvironment.mpi_ping_status_response_handler_service_sleep_time) 

            # Mark service as not running
            self.__ping_status_response_handler_service_running = False            

            
        def __start_ping_status_response_handler_service(self):
        
            casalog_call_origin = "MPIMonitorClient::start_ping_status_response_handler_service"

            if self.__ping_status_response_handler_service_running:
                casalog.post("MPI ping status response handler service is already running","WARN",casalog_call_origin)
                return True
            
            try:
                self.__ping_status_response_handler_service_on = True
                self.__ping_status_response_handler_service_thread = thread.start_new_thread(self.__ping_status_response_handler_service, ())
            except:
                formatted_traceback = traceback.format_exc()
                self.__ping_status_response_handler_service_on = False
                self.__ping_status_response_handler_service_running = False
                casalog.post("Exception starting MPI ping status response handler service: %s" 
                             % str(formatted_traceback),"SEVERE",casalog_call_origin)  
                return False
        
            while (not self.__ping_status_response_handler_service_running):
                time.sleep(MPIEnvironment.mpi_check_start_service_sleep_time) 
        
            casalog.post("MPI ping status response handler service started","INFO",casalog_call_origin)
            
            return True
        
        
        def __stop_ping_status_response_handler_service(self):
        
            casalog_call_origin = "MPIMonitorClient::stop_ping_status_response_handler_service"
        
            if not self.__ping_status_response_handler_service_running:
                casalog.post("MPI ping status response handler service is not running","WARN",casalog_call_origin)
                return             

            self.__ping_status_response_handler_service_on = False
        
            while (self.__ping_status_response_handler_service_running):
                time.sleep(MPIEnvironment.mpi_check_stop_service_sleep_time)
                
            casalog.post("MPI ping status response handler service stopped","INFO",casalog_call_origin)         
            
            
        ################################################################################################################            
        # Public methods ###############################################################################################
        ################################################################################################################
        
                    
        def start_services(self):
        
            self.__start_ping_status_response_handler_service()
            self.__start_monitor_status_service()
        
        
        def stop_services(self):

            self.__stop_monitor_status_service()
            self.__stop_ping_status_response_handler_service()            
            
            
        def get_server_status(self,server=None):
            
            casalog_call_origin = "MPIMonitorClient::get_server_status"
            
            if server is None:
                return dict(self.__server_status_list)
            else:
                if self.__server_status_list.has_key(server):
                    return dict(self.__server_status_list[server])
                else:
                    casalog.post("Server n# %s is out of range" % str(server),"WARN",casalog_call_origin)  
            
            
        def get_server_status_keyword(self,server,keyword):
            
            casalog_call_origin = "MPIMonitorClient::get_server_status_keyword"
            
            if self.__server_status_list.has_key(server):
                if self.__server_status_list[server].has_key(keyword):
                    return self.__server_status_list[server][keyword]
                else:
                    casalog.post("Status keyword %s not defined" % str(keyword),"WARN",casalog_call_origin)
            else:
                casalog.post("Server n# %s is out of range" % str(server),"WARN",casalog_call_origin)         
            
            
        def set_server_status_keyword(self,server,keyword,value):
            
            casalog_call_origin = "MPIMonitorClient::set_server_status_keyword"
            
            if self.__server_status_list.has_key(server):
                if self.__server_status_list[server].has_key(keyword):
                    self.__server_status_list[server][keyword]=value
                else:
                    casalog.post("Status keyword %s not defined" % str(keyword),"WARN",casalog_call_origin)
            else:
                casalog.post("Server n# %s is out of range" % str(server),"WARN",casalog_call_origin)
         
        
        def get_server_rank_available(self,verbose=False):
                     
            server_rank_available = []
            for rank in self.__server_status_list:
                if not (self.__server_status_list[rank]['busy'] or self.__server_status_list[rank]['timeout']):
                    server_rank_available.append(rank)
                
            return server_rank_available
        
        
        def get_server_rank_online(self,verbose=False):
                     
            server_rank_online = []
            for rank in self.__server_status_list:
                if not self.__server_status_list[rank]['timeout']:
                    server_rank_online.append(rank)
                
            return server_rank_online        
        
        
        def get_server_timeout(self):
            
            casalog_call_origin = "MPIMonitorClient::get_server_timeout"
            
            server_rank_timeout = []
            for rank in self.__server_status_list:
                if self.__server_status_list[rank]['timeout'] is True:
                    server_rank_timeout.append(rank)
                    
            return server_rank_timeout
        
        
# EOF
