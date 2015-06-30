#!/usr/bin/env python
import socket
import traceback # To pretty-print tracebacks
import os


class MPIEnvironment:

    # Static variables #################################################################################################

    # Set hostname
    hostname = socket.gethostname()

    # Initialization
    mpi_initialized = False
    try:
        # don't load mpi unless we are already running under mpi
        # trying to load a broken mpi installation will abort the process not
        # giving us a chance to run in the serial mode
        # testing mpi via a forked import causes deadlock on process end when
        # running test_mpi4casa[test_server_not_responsive]
        if 'OMPI_COMM_WORLD_RANK' not in os.environ:
            raise ValueError('MPI disabled')

        # Set mpi4py runtime configuration
        from mpi4py import rc as __mpi_runtime_config
        # Automatic MPI initialization at import time
        __mpi_runtime_config.initialize = True  
        # Request for thread support at MPI initialization
        __mpi_runtime_config.threaded = True  
        # Level of thread support to request at MPI initialization
        # "single" : use MPI_THREAD_SINGLE
        # "funneled" : use MPI_THREAD_FUNNELED
        # "serialized" : use MPI_THREAD_SERIALIZED
        # "multiple" : use MPI_THREAD_MULTIPLE
        __mpi_runtime_config.thread_level = 'multiple'
        # Automatic MPI finalization at exit time
        __mpi_runtime_config.finalize = False
        
        # Import mpi4py and thus initialize MPI
        from mpi4py import MPI as __mpi_factory # NOTE: This is a private variable to avoid uncontrolled access to MPI
        
        # Get world size and processor rank
        mpi_world_size = __mpi_factory.COMM_WORLD.Get_size()
        mpi_processor_rank = __mpi_factory.COMM_WORLD.Get_rank()
        
        # Get pid
        mpi_pid = os.getpid()
        
        # Get version and vendor info
        mpi_version_info = __mpi_factory.Get_version()
        mpi_vendor_info = __mpi_factory.get_vendor()
        mpi_thread_safe_level = __mpi_factory.Query_thread()
        
        # Prepare version info string
        mpi_version_str = str(mpi_version_info[0])
        for tuple_element_index in range(1,len(mpi_version_info)):
            mpi_version_str = mpi_version_str + "." + str(mpi_version_info[tuple_element_index])          
            
        # Prepare vendor info string
        mpi_vendor_str = str(mpi_vendor_info[0])
        if len(mpi_vendor_info)>1:
            mpi_vendor_version = mpi_vendor_info[1]
            mpi_vendor_version_str = str(mpi_vendor_version[0])
            for tuple_element_index in range(1,len(mpi_vendor_version)):
                mpi_vendor_version_str = mpi_vendor_version_str + "." + str(mpi_vendor_version[tuple_element_index])
            mpi_vendor_str = mpi_vendor_str + " v" + mpi_vendor_version_str
            
        # Set initialization flag
        mpi_initialized = True
    except Exception, instance:
        mpi_initialization_error_msg = traceback.format_exc()
        __mpi_factory = None
        mpi_world_size = 1
        mpi_processor_rank = 0
        mpi_version_info = None
        mpi_vendor_info = None
        mpi_version_str = ""
        mpi_vendor_str = ""
        mpi_vendor_version_str = ""
        mpi_thread_safe_level = -1
        mpi_initialized = False
        
    # Check if MPI implementation is thread safe
    if mpi_initialized:
        if mpi_thread_safe_level == __mpi_factory.THREAD_SINGLE:
            mpi_thread_safe_level_str = "MPI THREAD SINGLE"
            is_mpi_thread_safe = False
        elif mpi_thread_safe_level == __mpi_factory.THREAD_FUNNELED:
            mpi_thread_safe_level_str = "MPI THREAD FUNNELED"
            is_mpi_thread_safe = False
        elif mpi_thread_safe_level == __mpi_factory.THREAD_SERIALIZED:
            mpi_thread_safe_level_str = "MPI THREAD SERIALIZED"
            is_mpi_thread_safe = False
        elif mpi_thread_safe_level == __mpi_factory.THREAD_MULTIPLE:
            mpi_thread_safe_level_str = "MPI THREAD MULTIPLE"
            is_mpi_thread_safe = True
        else:
            mpi_thread_safe_level_str = str(mpi_thread_safe_level)
            is_mpi_thread_safe = False  
            
    # Generate MPI thread-safe info message
    if mpi_initialized and not is_mpi_thread_safe:
        mpi_thread_safe_info_msg = "Provided MPI implementation (%s) is not thread safe configured, " % (mpi_vendor_str)
        mpi_thread_safe_info_msg = mpi_thread_safe_info_msg +"maximum thread safe level supported is: %s" % (mpi_thread_safe_level_str)
        mpi_thread_safe_info_msg = mpi_thread_safe_info_msg + "\nNOTE: In most MPI implementations thread-safety "
        mpi_thread_safe_info_msg = mpi_thread_safe_info_msg + "can be enabled at pre-compile, "
        mpi_thread_safe_info_msg = mpi_thread_safe_info_msg + "by setting explicit thread-safe configuration options, "
        mpi_thread_safe_info_msg = mpi_thread_safe_info_msg + "\n      e.g. (MPI 1.6.5) --enable-mpi-thread-multiple"    
        
    # Allocate memory for buffered sends
    if mpi_initialized  and mpi_world_size > 1 and is_mpi_thread_safe:
        try:
            mpi_buffer_size_in_mb = 100
            __mpi_buffer = __mpi_factory.Alloc_mem(mpi_buffer_size_in_mb*1024*1024)
            __mpi_factory.Attach_buffer(__mpi_buffer)
            mpi_buffer_allocated = True
        except Exception, instance:
            mpi_buffer_allocated = False
            mpi_buffer_allocation_error_msg = traceback.format_exc()
        
    # Check if MPI is effectively enabled
    if mpi_initialized  and mpi_world_size > 1 and is_mpi_thread_safe and mpi_buffer_allocated:
        is_mpi_enabled = True           
    else:
        is_mpi_enabled = False 
        if not mpi_initialized:
            mpi_error_msg = mpi_initialization_error_msg
        elif mpi_world_size < 2:
            mpi_error_msg = "Only 1 MPI process found"
        elif not is_mpi_thread_safe:
            mpi_error_msg = mpi_thread_safe_info_msg   
        elif not mpi_buffer_allocated:
            mpi_error_msg = mpi_buffer_allocation_error_msg
        
    # Determine whether this processor is the 'privileged' MPI rank
    mpi_client_rank = 0
    if mpi_processor_rank == mpi_client_rank:
        is_mpi_client = True
        mpi_execution_role = "MPIClient"
    else:
        is_mpi_client = False
        # MPIServer is written in the C++ level
        mpi_execution_role = ""
        
    # Generate MPI info message
    mpi_info_msg = "MPI Enabled at host %s with rank %s as %s " % (hostname,mpi_processor_rank,mpi_execution_role)
    mpi_info_msg = mpi_info_msg + "using MPI version %s from %s implementation " % (mpi_version_str,mpi_vendor_str)             

    # If MPI is enabled populate list with the ranks of the MPI servers
    __mpi_server_rank_list = [] # NOTE: This is a private variable because Python list copy operator is by reference
    if is_mpi_enabled:
        for rank in range(mpi_world_size):
            if rank != mpi_client_rank:
                __mpi_server_rank_list.append(rank)               
                
    # Set showconsole settings
    if ((is_mpi_enabled) and (not is_mpi_client)):
        log_to_console = False
    else:
        log_to_console = True
                        
    # Generate the processor origin for the logger
    processor_origin = ""
    if is_mpi_enabled:
        if is_mpi_client:
            processor_origin = "@" + hostname + ":MPIClient"
        else:
            # Rank information of server is written in the C++ level
            processor_origin = "@" + hostname   
            
    # Set pre-determined log level
    command_handling_log_level = "NORMAL"
            
    # Set ANY_SOURCE and ANY_TAG constants
    if is_mpi_enabled:
        mpi_any_tag = __mpi_factory.ANY_TAG
        mpi_any_source = __mpi_factory.ANY_SOURCE
    else:
        mpi_any_tag = -1        
        mpi_any_source = -1             
                
    # Set progression profile
    if is_mpi_enabled:
        mpi_start_service_sleep_time = 0.1 # Aggressive, not concurrent with command execution (at the server)
        mpi_stop_service_sleep_time = 3 # Not aggressive, concurrent with command execution (at the server)
        mpi_check_start_service_sleep_time = 0.1 # Aggressive, temporal and not concurrent with command execution (at the client)
        mpi_check_stop_service_sleep_time = 0.1 # Aggressive, temporal and not concurrent with command execution (at the client)
        mpi_monitor_status_service_heartbeat = 5 # Not aggressive, permanent and concurrent with command execution (at the client)
        mpi_ping_status_request_handler_service_sleep_time = 3  # Not aggressive, concurrent with command execution (at the server)
        mpi_ping_status_response_handler_service_sleep_time = 3 # Not aggressive, concurrent with command execution (at the client)
        mpi_command_request_handler_service_sleep_time = 0.1 # Aggressive, not concurrent with command execution (at the server)
        mpi_command_response_handler_service_sleep_time = 0.1 # Aggressive, triggered (at the client)
        mpi_command_request_queue_service_sleep_time = 0.1 # Aggressive, triggered (at the client)
        mpi_push_command_request_block_mode_sleep_time = 0.1 # Aggressive, used for getting response in blocking mode (at the client)
        
        mpi_monitor_status_service_timeout = 1 # 2*[bsend+(Iprobe+recv) + serialization/deserialization + latency + locks]
        mpi_monitor_status_service_timeout += mpi_ping_status_request_handler_service_sleep_time # Sleep time at the server
        mpi_monitor_status_service_timeout += mpi_world_size-1 # Receive sequentially response from all servers
        mpi_monitor_status_service_timeout += mpi_ping_status_response_handler_service_sleep_time # Sleep time at the client
        mpi_monitor_status_service_timeout += mpi_monitor_status_service_heartbeat # Own heartbeat sleep time
        mpi_monitor_status_service_timeout *= 3 # 3 times what it should take
        
        mpi_ping_status_request_handler_service_timeout = 0.5 # 1*[bsend+(Iprobe+recv) + serialization/deserialization + latency + locks]
        mpi_ping_status_request_handler_service_timeout += mpi_world_size-1 # Receive sequentially response from all servers
        mpi_ping_status_request_handler_service_timeout += mpi_ping_status_response_handler_service_sleep_time # Sleep time at the client
        mpi_ping_status_request_handler_service_timeout += mpi_monitor_status_service_heartbeat # Own heartbeat sleep time
        mpi_ping_status_request_handler_service_timeout *= 3 # 3 times what it should take
    
    # Static methods ###################################################################################################       
        
    @staticmethod
    def mpi_comm_world_factory():
        return MPIEnvironment.__mpi_factory.COMM_WORLD.Dup()
    
    
    @staticmethod
    def mpi_server_rank_list():
        return list(MPIEnvironment.__mpi_server_rank_list) # Careful, Python list copy operator is by reference
    
    @staticmethod
    def finalize_mpi_environment():
        MPIEnvironment.__mpi_factory.Finalize()
        
# EOF

