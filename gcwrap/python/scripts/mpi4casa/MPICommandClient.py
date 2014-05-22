#!/usr/bin/env python
import thread # To handle service threads like monitoring
import time # To handle sleep times
import traceback # To pretty-print tracebacks
import atexit # To handle destructors

 # Import casalog and casa dictionary
from taskinit import *

# Import MPIEnvironment static class
from MPIEnvironment import MPIEnvironment

# Import MPICommunicator singleton
from MPICommunicator import MPICommunicator

# Import MPIMonitorClient singleton
from MPIMonitorClient import MPIMonitorClient
        

class MPICommandClient:
    
    
    # Storage for the MPICommandClient singleton instance reference
    __instance = None


    def __init__(self,start_services=False):
        """ Create MPICommandClient singleton instance """
        
        casalog_call_origin = "MPICommandClient::__init__"
               
        # Check if MPI is effectively enabled
        if not MPIEnvironment.is_mpi_enabled:
            msg = "MPI is not enabled"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check if MPICommandClient can be instantiated
        if not MPIEnvironment.is_mpi_client:
            msg = "MPICommandClient can only be instantiated at master MPI process"
            casalog.post(msg,"SEVERE",casalog_call_origin)
            raise Exception,msg
        
        # Check whether we already have a MPICommandClient singleton instance
        if MPICommandClient.__instance is None:
            # Create MPICommandClient singleton instance
            MPICommandClient.__instance = MPICommandClient.__MPICommandClientImpl(start_services=start_services)

        # Store MPICommandClient singleton instance reference as the only member in the handle
        self.__dict__['_MPICommandClient__instance'] = MPICommandClient.__instance
        
    def __getattr__(self, attr):
        """ Delegate access to MPICommandClient implementation """
        return getattr(self.__instance, attr)


    def __setattr__(self, attr, value):
        """ Delegate access to MPICommandClient implementation """
        return setattr(self.__instance, attr, value)    
    
    
    class __MPICommandClientImpl:
        """ Implementation of the MPICommandClient singleton interface """
        
        
        def __init__(self,start_services=False):
            
            # Initialize life cycle state
            # 0-> Services not started
            # 1-> Services started
            # 2-> Stop service signal sent
            self.__life_cycle_state = 0
             
                    
            # Initialize command request id counter and list
            self.__command_request_counter = 1
            self.__command_request_list = {}
            self.__command_response_list = {}
            
            # Initialize command response handler service state
            self.__command_response_handler_service_on = False
            self.__command_response_handler_service_running = False
            self.__command_response_handler_service_thread = None 
            
            # Initialize command request queue service state
            self.__command_request_queue = []
            self.__command_request_queue_service_on = False
            self.__command_request_queue_service_running = False
            self.__command_request_queue_service_thread = None             
            
            # Instantiate MPICommunicator reference
            self.__communicator = MPICommunicator()
            
            # Instantiate MPIMonitorClient reference
            self.__monitor_client = MPIMonitorClient(False)            
            
            # Automatically start services
            if start_services:
                self.start_services()
                
            # Register exit handler
            # NOTE: It is not guaranteed that __del__() methods are called 
            #       for objects that still exist when the interpreter exits.
            atexit.register(self.stop_services,force_command_request_interruption=True)
                                
                                   
        ################################################################################################################            
        # Private methods ##############################################################################################
        ################################################################################################################                                   
            
            
        def __command_response_handler_service(self):
            
            casalog_call_origin = "MPICommandClient::command_response_handler_service"
            
            # Mark service as running
            self.__command_response_handler_service_running = True            
                          
            while (self.__command_response_handler_service_on):
                
                # First check if there is a command response msg available
                msg_available = False
                try:
                    msg_available = self.__communicator.command_response_probe()
                except Exception, instance:
                    msg_available = False
                    formatted_traceback = traceback.format_exc()
                    casalog.post("Exception checking if command response msg is available: %s" 
                                 % str(formatted_traceback),"SEVERE",casalog_call_origin)                 
                    
                # Then receive, store and post command response msg
                if (msg_available):            
                    try:
                        # Receive command response
                        command_response = self.__communicator.command_response_recv()
                        server = command_response['server']
                        successful = command_response['successful']
                        command_id = command_response['id']
                        # Mark immediately server as not-busy
                        self.__monitor_client.set_server_status_keyword(server,'busy',False)
                        # Store command response
                        self.__command_response_list[command_id] = command_response 
                        # Mark command request as received
                        self.__command_request_list[command_id]['status'] = 'response received' 
                        self.__command_response_list[command_id]['status'] = 'response received' 
                        # Notify that command response has been received
                        if successful:
                            casalog.post("Command request with id %s successfully handled by server n# %s" 
                                         % (str(command_id),str(server)),"INFO",casalog_call_origin)                                  
                        else:
                            casalog.post("Command request with id %s failed in server n# %s with traceback %s" 
                                         % (str(command_id),str(server),str(command_response['traceback'])),
                                         "INFO",casalog_call_origin)                            
                    except Exception, instance:
                        formatted_traceback = traceback.format_exc()
                        casalog.post("Exception receiving command request response msg: %s" 
                                     % str(formatted_traceback),"SEVERE",casalog_call_origin)
                else:
                    time.sleep(MPIEnvironment.mpi_command_response_handler_service_sleep_time) 

            # Mark service as not running
            self.__command_response_handler_service_running = False            

            
        def __start_command_response_handler_service(self):
        
            casalog_call_origin = "MPICommandClient::start_command_response_handler_service"

            if self.__command_response_handler_service_running:
                casalog.post("MPI command response handler service is already running","WARN",casalog_call_origin)
                return True
            
            try:
                self.__command_response_handler_service_on = True
                self.__command_response_handler_service_thread = thread.start_new_thread(self.__command_response_handler_service, ())
            except Exception, instance:
                formatted_traceback = traceback.format_exc()
                self.__command_response_handler_service_on = False
                self.__command_response_handler_service_running = False
                casalog.post("Exception starting MPI command response handler service: %s" 
                             % str(formatted_traceback),"SEVERE",casalog_call_origin)  
                return False
        
            while (not self.__command_response_handler_service_running):
                time.sleep(MPIEnvironment.mpi_check_start_service_sleep_time) 
        
            casalog.post("MPI command response handler service started","INFO",casalog_call_origin)
            
            return True
        
        
        def __stop_command_response_handler_service(self):
        
            casalog_call_origin = "MPICommandClient::stop_command_response_handler_service"
        
            if not self.__command_response_handler_service_running:
                casalog.post("MPI command response handler service is not running","WARN",casalog_call_origin)
                return             

            self.__command_response_handler_service_on = False
        
            while (self.__command_response_handler_service_running):
                time.sleep(MPIEnvironment.mpi_check_stop_service_sleep_time)
                
            casalog.post("MPI command response handler service stopped","INFO",casalog_call_origin)
            
            
        def __command_request_queue_service(self):
            
            casalog_call_origin = "MPICommandClient::command_request_queue_service"
            
            # Mark service as running
            self.__command_request_queue_service_running = True                    
                       
            while self.__command_request_queue_service_on:
                # Get list of available servers
                available_servers_list = self.__monitor_client.get_server_rank_available()
                if len(available_servers_list) >= 1:
                    # Get list of matching command requests
                    matching_command_request_id_list = self.__match_available_servers_with_command_requests(available_servers_list)
                    # Iterate over matching command request list
                    for command_request_id in matching_command_request_id_list:
                        command_request_found = False
                        command_request_queue_idx = 0
                        # Iterate over command request queue to find the corresponding index
                        for command_request in self.__command_request_queue:
                            if command_request['id'] == command_request_id:
                                command_request_found = True
                                break
                            else:
                                command_request_queue_idx = command_request_queue_idx + 1
                        # Extract command request from queue and send it
                        if command_request_found:
                            command_request = self.__command_request_queue.pop(command_request_queue_idx)
                            server = command_request['server']
                            try:
                                # Mark assigned server as busy and set command info in server status
                                self.__monitor_client.set_server_status_keyword(server,'busy',True)
                                self.__monitor_client.set_server_status_keyword(server,'command',command_request['command'])
                                # Send command request
                                self.__communicator.command_request_send(request=command_request,server=server)
                                # Mark command request as sent
                                self.__command_request_list[command_request_id]['status']='request sent'
                                # Notify that command request has been sent
                                casalog.post("Command request with id# %s sent to server n# %s" 
                                             % (str(command_request_id),str(server)),"INFO",casalog_call_origin)
                            except Exception, instance:
                                # Get and format traceback
                                formatted_traceback = traceback.format_exc()
                                # Simulate response
                                command_response = dict(command_request)
                                command_response['successful']=False
                                command_response['traceback']=formatted_traceback
                                self.__command_response_list[command_request_id]=command_response
                                # Notify exception
                                casalog.post("Exception sending command request with id# %s to server n# %s: %s"
                                             % (str(command_request_id),str(server),str(formatted_traceback)),
                                             "SEVERE",casalog_call_origin)                       
                        else:
                            casalog.post("Command request with id# %s not found" % 
                                         str(command_request_id),"SEVERE",casalog_call_origin)
                else:
                    # Sleep in order not to saturate the system
                    time.sleep(MPIEnvironment.mpi_command_request_queue_service_sleep_time)
                
            # Mark service as not running
            self.__command_request_queue_service_running = False
            
            
        def __match_available_servers_with_command_requests(self,available_servers):
            
            matching_command_request_id_list = []
            unassigned_command_request_id_list = []
            available_servers_left = list(available_servers)
            for command_request in self.__command_request_queue:
                server = command_request['server']
                command_request_id = command_request['id']
                # Command request does not have any pre-assigned server
                if server is None:
                    matching_command_request_id_list.append(command_request_id)
                    unassigned_command_request_id_list.append(command_request_id)
                # Assigned server is within the list of available servers 
                elif server in available_servers_left:
                    matching_command_request_id_list.append(command_request_id)
                    # Remove server from the list of available servers to avoid multiple assignment
                    available_servers_left.remove(server)
                    
                # Exit loop if we have enough matching requests
                if len(matching_command_request_id_list) >= len(available_servers):
                    break
                
            # Assign servers to the remaining requests
            for command_request in self.__command_request_queue:
                if command_request['id'] in unassigned_command_request_id_list:
                    server = available_servers_left.pop()
                    command_request['server'] = server
                
            # Return matching command request id list
            return matching_command_request_id_list  
        
            
        def __start_command_request_queue_service(self):
        
            casalog_call_origin = "MPICommandClient::start_command_request_queue_service"

            if self.__command_request_queue_service_running:
                casalog.post("MPI command request queue service is already running","WARN",casalog_call_origin)
                return True
            
            try:
                self.__command_request_queue_service_on = True
                self.__command_request_queue_service_thread = thread.start_new_thread(self.__command_request_queue_service, ())
            except Exception, instance:
                formatted_traceback = traceback.format_exc()
                self.__command_request_queue_service_on = False
                self.__command_request_queue_service_running = False
                casalog.post("Exception starting MPI command request queue service: %s" 
                             % str(formatted_traceback),"SEVERE",casalog_call_origin)  
                return False
        
            while (not self.__command_request_queue_service_running):
                time.sleep(MPIEnvironment.mpi_check_start_service_sleep_time) 
        
            casalog.post("MPI command request queue service started","INFO",casalog_call_origin)
            
            return True
        
        
        def __stop_command_request_queue_service(self):
        
            casalog_call_origin = "MPICommandClient::stop_command_request_queue_service"
        
            if not self.__command_request_queue_service_running:
                casalog.post("MPI command request queue service is not running","WARN",casalog_call_origin)
                return             

            self.__command_request_queue_service_on = False
        
            while (self.__command_request_queue_service_running):
                time.sleep(MPIEnvironment.mpi_check_stop_service_sleep_time)
                
            casalog.post("MPI command request queue service stopped","INFO",casalog_call_origin)            
            
            
        def __send_start_service_signal(self):
            
            casalog_call_origin = "MPICommandClient::send_start_service_signal"
            
            casalog.post("Sending start service signal to all servers","INFO",casalog_call_origin)
            
            # Prepare stop service request
            request = {}
            request['signal'] = 'start'
            request['casa'] = casa # The request contains the global casa dictionary to be used by the servers
            
            # Send request to all servers
            self.__communicator.control_service_request_broadcast(request,casalog)
                
            # Then wait until all servers have handled the signal
            mpi_server_rank_list = MPIEnvironment.mpi_server_rank_list()
            while len(mpi_server_rank_list)>0:
                response_available = False
                response_available = self.__communicator.control_service_response_probe()
                if response_available:
                    # Receive start service response to know what server has started
                    response = self.__communicator.control_service_response_recv()
                    rank = response['rank']
                    # Store processor name and PID info in the MPIMonitorClient
                    self.__monitor_client.set_server_status_keyword(rank,'processor',response['processor'])
                    self.__monitor_client.set_server_status_keyword(rank,'pid',response['pid'])
                    # Remove server from list
                    mpi_server_rank_list.remove(rank)
                    # Communicate that server response to start service signal has been received
                    casalog.post("Server with rank %s started at %s with PID %s" 
                                 % (str(rank),str(response['processor']),str(response['pid'])),
                                 "INFO",casalog_call_origin)
                else:
                    time.sleep(MPIEnvironment.mpi_check_start_service_sleep_time)
                        
            casalog.post("Received response from all servers to start service signal","INFO",casalog_call_origin)
            
        
        def __send_stop_service_signal(self,force_command_request_interruption=False):
            
            casalog_call_origin = "MPICommandClient::send_stop_service_signal"
            
            casalog.post("Sending stop service signal to all servers","INFO",casalog_call_origin)
            
            # Prepare stop service request
            request = {}
            request['signal'] = 'stop'
            request['force_command_request_interruption'] = force_command_request_interruption
            
            # Send request to all servers
            self.__communicator.control_service_request_broadcast(request,casalog)
            
            # Register servers as not running in MPICommunicator
            self.__communicator.set_servers_running(False)
            
            casalog.post("Stop service signal sent to all servers","INFO",casalog_call_origin)
            
            
        def __validate_target_servers(self,target_server):
            
            casalog_call_origin = "MPICommandClient::validate_target_servers"
            
            # Get list of valid MPIServer ranks
            mpi_server_rank_list = MPIEnvironment.mpi_server_rank_list()
            
            # Check if target server is a list of integers
            if isinstance(target_server,list) and (len(target_server)>=1) and all(isinstance(server, int) for server in target_server):
                # Check if server is within the server rank list
                for server in target_server:
                    if server not in mpi_server_rank_list:
                        casalog.post("Server #%s does not exist" % str(server),"SEVERE",casalog_call_origin)
                        return None
                    elif self.__monitor_client.get_server_status_keyword(server,'timeout'):
                        casalog.post("Server #%s has timed out" % str(server),"SEVERE",casalog_call_origin)
                        return None
                # Return input list validated
                return target_server
            # Check if target server is an integer
            elif isinstance(target_server,int):
                # Check if server is within the server rank list
                if target_server in mpi_server_rank_list:
                    return [target_server]
                else:
                    casalog.post("Server #%s does not exist" % str(target_server),"SEVERE",casalog_call_origin)
                    return None
            else:
                casalog.post("target_server has wrong format (%s), accepted formats are int and list(int)" 
                             % str(type(target_server)),"SEVERE",casalog_call_origin)                
                return None        
            
            
        def __register_command_request(self,command_request,server):
            
            # Get command request if
            command_request_id = self.__command_request_counter 
            
            # Complete command request definition
            command_request_complete = dict(command_request)
            command_request_complete['id'] = command_request_id
            command_request_complete['server'] = server
            command_request_complete['status'] = 'holding queue'
                       
            # Register command request
            self.__command_request_list[command_request_id]=command_request_complete
             
            # Queue command request
            self.__command_request_queue.append(command_request_complete)
             
            # Increment command id counter
            self.__command_request_counter = self.__command_request_counter + 1    
            
            # Return command request id
            return command_request_id     
        
        def __format_command_response_timeout(self,command_request_id):
            
            # Create a fake command response copying the command request and marking it as not successful
            command_response = dict(self.__command_request_list[command_request_id])
            command_response['status']='timeout'
            command_response['successful']=False
            command_response['ret']=None
            
            # Get server, processor and pid to identify which server timed out
            server = command_response['server']
            processor = self.__monitor_client.get_server_status_keyword(server,'processor')
            pid = self.__monitor_client.get_server_status_keyword(server,'pid')
                        
            # Create command response trace-back msg
            timeout_msg = "Timeout of assigned server n# " + str(server)
            timeout_msg = timeout_msg + " deployed at " + str(processor)
            timeout_msg = timeout_msg + " with PID " + str(pid)
                        
            command_response['traceback'] = timeout_msg
            
            return command_response
            
        ################################################################################################################            
        # Public methods ###############################################################################################
        ################################################################################################################
            
            
        def start_services(self):
            
            casalog_call_origin = "MPICommandClient::start_services"
            
            if self.__life_cycle_state == 1:
                casalog.post("Services already started","WARN",casalog_call_origin)
                return       
            elif self.__life_cycle_state == 2:
                casalog.post("MPICommandClient life cycle finalized","WARN",casalog_call_origin)
                return 
        
            # 1st: start servers
            self.__send_start_service_signal()
            
            # 2nd: start monitoring servers
            self.__monitor_client.start_services()
            
            # 3rd: start command request queue service
            self.__start_command_request_queue_service()
            
            # 4th: start command response handler service
            self.__start_command_response_handler_service()
            
            # Set life cycle state
            self.__life_cycle_state = 1
            
            casalog.post("All services started","INFO",casalog_call_origin)
        
        
        def stop_services(self,force_command_request_interruption=False):

            casalog_call_origin = "MPICommandClient::stop_services"
            
            if self.__life_cycle_state == 0:
                casalog.post("Services not started","WARN",casalog_call_origin)
                return       
            elif self.__life_cycle_state == 2:
                casalog.post("MPICommandClient life cycle finalized","WARN",casalog_call_origin)
                return             
            
            # Stop client monitoring services
            self.__monitor_client.stop_services()
            
            # Notify command requests which are going to be interrupted
            for command_request_id in self.__command_request_list:
                    if not self.__command_response_list.has_key(command_request_id):
                        server = self.__command_request_list[command_request_id]['server']
                        status = self.__command_request_list[command_request_id]['status']
                        casalog.post("Aborting command request with id# %s: %s" 
                                     % (str(command_request_id),str(self.__command_request_list[command_request_id])),
                                     "SEVERE",casalog_call_origin)
            
            # Stop client command request-response services
            self.__stop_command_request_queue_service()
            self.__stop_command_response_handler_service()          
            
            # Send stop signal to servers  
            self.__send_stop_service_signal(force_command_request_interruption)
            
            # Set life cycle state
            self.__life_cycle_state = 2            
            
            casalog.post("All services stopped","INFO",casalog_call_origin)
           

        def push_command_request(self,command,block=False,target_server=None,parameters=None):
        
            casalog_call_origin = "MPICommandClient::push_command_request"
            
            if self.__life_cycle_state == 0:
                casalog.post("Services not started","WARN",casalog_call_origin)
                return       
            elif self.__life_cycle_state == 2:
                casalog.post("MPICommandClient life cycle finalized","WARN",casalog_call_origin)
                return  
            
            command_request = {}
            command_request['command']=command
            command_request['parameters'] = parameters
            
            # Determine whether command is a statement or an expression
            try:
                code = compile(command_request['command'],"send_command_request", "eval")
                command_request['mode']='eval'
                casalog.post("Command will be evaluated as an expression with return value",
                             "DEBUG",casalog_call_origin)                
            except Exception, instance:
                try:
                    code = compile(command_request['command'],"send_command_request", "exec")
                    command_request['mode']='exec'
                    casalog.post("Command will be executed as an statement w/o return code",
                                 "DEBUG",casalog_call_origin)                    
                except Exception, instance:
                     formatted_traceback = traceback.format_exc()
                     casalog.post("Command cannot be executed neither as a statement nor as an expression, it will be rejected" 
                                  % str(formatted_traceback),"SEVERE",casalog_call_origin)
                     return None   
            
            # Validate target servers
            target_server_validated = None
            if target_server is not None:
                target_server_validated = self.__validate_target_servers(target_server)
                # Exit if target server is not validated
                if target_server_validated is None:
                    return None
                
            # Create command request list
            command_request_id_list = []
            if target_server_validated is not None:
                for server in target_server_validated:
                    command_request_id = self.__register_command_request(command_request,server)
                    command_request_id_list.append(command_request_id)
            else:
                command_request_id = self.__register_command_request(command_request,None)
                command_request_id_list.append(command_request_id)
            
            # In blocking mode wait until command response is received otherwise return request id
            if block:
                command_return_code_list = self.get_command_response(command_request_id_list,True,False)
                return command_return_code_list
            # Otherwise we simply return the command request id
            else:
                return command_request_id_list
        
        
        def get_command_response(self,command_request_id_list,block=False,verbose=True):
            
            casalog_call_origin = "MPICommandClient::get_command_response"    
            
            
            command_response_list = []
            if block:
                
                # Wait until command request response is received or timeout
                pending_command_request_id_list = list(command_request_id_list)
                while len(pending_command_request_id_list)>0:
                    for command_request_id in command_request_id_list:
                        # Check if command request id is still pending
                        if command_request_id in pending_command_request_id_list:
                            # Check if we have response for command request id
                            if self.__command_response_list.has_key(command_request_id):
                                # Remove command request id from pending list
                                pending_command_request_id_list.remove(command_request_id)
                            else:
                                server = self.__command_request_list[command_request_id]['server']
                                if server is not None and self.__monitor_client.get_server_status_keyword(server,'timeout'):
                                    casalog.post("Command request with id# %s sent to server n# %s, but the server has timed out" 
                                                 % (str(command_request_id),str(server)),"SEVERE",casalog_call_origin)
                                    # Remove command request id from pending list
                                    pending_command_request_id_list.remove(command_request_id)
                                
                            
                    time.sleep(MPIEnvironment.mpi_push_command_request_block_mode_sleep_time)
                    
                # Gather command response list
                for command_request_id in command_request_id_list:
                    if self.__command_response_list.has_key(command_request_id):
                        command_response = dict(self.__command_response_list[command_request_id])
                        command_response_list.append(command_response)
                    else:
                        command_response = self.__format_command_response_timeout(command_request_id)
                        command_response_list.append(command_response)
                    
                # Gather return codes
                #command_return_code_list = []
                #for command_response in command_response_list:
                #    successful = command_response['successful']
                #    if not successful:
                #        command_return_code_list.append([command_response['id'],False, command_response['traceback']])
                #    elif command_response['mode'] == 'eval':
                #        command_return_code_list.append([command_response['id'],True,command_response['ret']])
                #    else:
                #        command_return_code_list.append([command_response['id'],True,None])
                
                # Return command return code list
                return command_response_list
            
            else:
                command_response_list = []
                for command_request_id in command_request_id_list:
                    if not self.__command_response_list.has_key(command_request_id):
                        if verbose:
                            status = self.__command_request_list[command_request_id]['status']
                            if status == 'holding queue':
                                casalog.post("Command request with id# %s is still holding a queue" 
                                             % str(command_request_id),"INFO",casalog_call_origin)
                            else:
                                server = self.__command_request_list[command_request_id]['server']
                                timeout = self.__monitor_client.get_server_status_keyword(server,'timeout')
                                if timeout:
                                    casalog.post("Command request with id# %s sent to server n# %s, but the server has timed out" 
                                             % (str(command_request_id),str(server)),"SEVERE",casalog_call_origin)
                                    command_response = self.__format_command_response_timeout(command_request_id)
                                    command_response_list.append(command_response)
                                else:
                                    casalog.post("Command request with id# %s assigned to server n# %s, response pending ..." 
                                             % (str(command_request_id),str(server)),"INFO",casalog_call_origin)
                    else:
                        command_response = dict(self.__command_response_list[command_request_id])
                        command_response_list.append(command_response)
                        
                return command_response_list
            
        
        def get_server_status(self,server=None):
            return self.__monitor_client.get_server_status(server)
   
# EOF