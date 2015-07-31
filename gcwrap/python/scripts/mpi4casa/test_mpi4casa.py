import os
import sys
import time
import signal
import socket
import traceback
import unittest
import testhelper
import filecmp
from taskinit import mstool,tbtool,cbtool,casalog,casac,casa
from tasks import setjy,flagdata,applycal,uvcontsub
from mpi4casa.MPIEnvironment import MPIEnvironment
from mpi4casa.MPICommandClient import MPICommandClient
from mpi4casa.MPIMonitorClient import MPIMonitorClient
from mpi4casa.MPICommandServer import MPICommandServer
from mpi4casa.MPIInterface import MPIInterface
from parallel.parallel_task_helper import ParallelTaskHelper
from parallel.parallel_task_helper import ParallelTaskWorker


def waitForFile( file, seconds):
        
    for i in range(0,seconds):
        if (os.path.isfile(file)):
            return
        time.sleep(1)
            
def create_input(str_text, filename):
    """Save the string in a text file"""
    
    inp = filename
    cmd = str_text
    
    # Remove file first
    if os.path.exists(inp):
        os.system('rm -f '+ inp)
        
    # Save to a file    
    fid = open(inp, 'w')
    fid.write(cmd)
        
    # Close file
    fid.close()

    # Wait until file is visible for the file system
    waitForFile(filename, 10)
    
    return

def setUpFileCore(file,type_file):
        
    if os.path.exists(file):
        casalog.post("%s file %s is already in the working area, deleting ..." % (type_file,file),"WARN","test_mpi4casa")
        os.system('rm -rf ' + file)

    casalog.post("Copy %s file %s into the working area..." % (type_file,file),"INFO","test_mpi4casa")
    os.system('cp -RL ' + os.environ.get('CASAPATH').split()[0] + 
              '/data/regression/unittest/simplecluster/' + file + ' ' + file)

def setUpFile(file,type_file):
        
    if type(file) is list:
        for file_i in file:
            setUpFileCore(file_i,type_file)
    else:
        setUpFileCore(file,type_file)
        
def sortFile(input_file,output_file,sort_order=None):
    
    if sort_order is None:
        sort_order = ['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME']
        
    mymstool = mstool()
    mymstool.open(input_file)
    mymstool.sort(output_file,sort_order)
    mymstool.done()
    

class test_MPICommandClient(unittest.TestCase):
       
    def setUp(self):
        
        self.client = MPICommandClient()
        self.client.set_log_mode('redirect')
        self.server_list = MPIEnvironment.mpi_server_rank_list()
        self.client.start_services()
                    
    def test_exec_undefined_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(3)",True,None)
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_defined_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(3)",True,self.server_list[0])
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_exec_multiple_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(3)",True,[self.server_list[0],self.server_list[1]])
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_exec_undefined_target_non_blocking_mode_str_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(3)",False,None)
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_defined_target_non_blocking_mode_str_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(3)",False,[self.server_list[0]])
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_multiple_target_non_blocking_mode_str_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(3)",False,[self.server_list[0],self.server_list[1]])
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_eval_undefined_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("1+1",True,None)
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 2, "Command return variable should be 2")
        
    def test_eval_defined_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("1+1",True,[self.server_list[0]])
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 2, "Command return variable should be 2")  
        
    def test_eval_multiple_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("1+1",True,[self.server_list[0],self.server_list[1]])
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], 2, "Command return variable should be 2")
            
    def test_eval_undefined_target_non_blocking_mode_str_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("1+1",False,None)
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 2, "Command return variable should be 2")
        
    def test_eval_defined_target_non_blocking_mode_str_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("1+1",False,[self.server_list[0]])
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 2, "Command return variable should be 2")
        
    def test_eval_multiple_target_non_blocking_mode_str_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("1+1",False,[self.server_list[0],self.server_list[1]])
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], 2, "Command return variable should be None")         

    def test_exec_undefined_target_blocking_mode_dict_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a+b)",True,None,{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_defined_target_blocking_mode_dict_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a+b)",True,[self.server_list[0]],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_exec_multiple_target_blocking_mode_dict_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a+b)",True,[self.server_list[0],self.server_list[1]],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_exec_undefined_target_non_blocking_mode_dict_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a+b)",False,None,{'a':1,'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_defined_target_non_blocking_mode_dict_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a+b)",False,[self.server_list[0]],{'a':1,'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_multiple_target_non_blocking_mode_dict_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a+b)",False,[self.server_list[0],self.server_list[1]],{'a':1,'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_eval_undefined_target_blocking_mode_dict_params_successful(self):
        
        command_response_list = self.client.push_command_request("a+b",True,None,{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 3, "Command return variable should be 3")
        
    def test_eval_defined_target_blocking_mode_dict_params_successful(self):
        
        command_response_list = self.client.push_command_request("a+b",True,[self.server_list[0]],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 3, "Command return variable should be 3")  
        
    def test_eval_multiple_target_blocking_mode_dict_params_successful(self):
        
        command_response_list = self.client.push_command_request("a+b",True,[self.server_list[0],self.server_list[1]],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], 3, "Command return variable should be 3")
            
    def test_eval_undefined_target_non_blocking_mode_dict_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("a+b",False,None,{'a':1,'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 3, "Command return variable should be 3")
        
    def test_eval_defined_target_non_blocking_mode_dict_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("a+b",False,[self.server_list[0]],{'a':1,'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 3, "Command return variable should be 3")
        
    def test_eval_multiple_target_non_blocking_mode_dict_params_successful(self):
        
        command_request_id_list = self.client.push_command_request("a+b",False,[self.server_list[0],self.server_list[1]],{'a':1,'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], 3, "Command return variable should be 3")

    def test_exec_undefined_target_blocking_mode_str_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("import something; something.do()",True,None)
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("ImportError:")>=0, True, "Trace-back should contain ImportError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")            
        
    def test_exec_defined_target_blocking_mode_str_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a)",True,[self.server_list[0]])
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_exec_multiple_target_blocking_mode_str_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(3/0)",True,[self.server_list[0],self.server_list[1]])
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], False, "Command execution was successful")
            self.assertEqual(command_response['traceback'].find("ZeroDivisionError:")>=0, True, "Trace-back should contain ZeroDivisionError")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_exec_undefined_target_non_blocking_mode_str_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep('a')",False,None)
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("TypeError:")>=0, True, "Trace-back should contain TypeError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_defined_target_non_blocking_mode_str_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("test=[];test[1]=2",False,[self.server_list[0]])
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("IndexError:")>=0, True, "Trace-back should contain IndexError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_multiple_target_non_blocking_mode_str_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("test={};test['bananas']",False,[self.server_list[0],self.server_list[1]])
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], False, "Command execution was successful")
            self.assertEqual(command_response['traceback'].find("KeyError:")>=0, True, "Trace-back should contain KeyError")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_eval_undefined_target_blocking_mode_str_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("test=__import__('something')",True,None)
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("ImportError:")>=0, True, "Trace-back should contain ImportError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_eval_defined_target_blocking_mode_str_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("a+1",True,[self.server_list[0]])
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_eval_multiple_target_blocking_mode_str_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("1/0",True,[self.server_list[0],self.server_list[1]])
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], False, "Command execution was successful")
            self.assertEqual(command_response['traceback'].find("ZeroDivisionError:")>=0, True, "Trace-back should contain ZeroDivisionError")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_eval_undefined_target_non_blocking_mode_str_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("pow('something',3)",False,None)
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("TypeError:")>=0, True, "Trace-back should contain TypeError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_eval_defined_target_non_blocking_mode_str_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("test=[0,1]",True,[self.server_list[0]])
        command_request_id_list = self.client.push_command_request("test[3]",False,[self.server_list[0]])
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("IndexError:")>=0, True, "Trace-back should contain IndexError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_eval_multiple_target_non_blocking_mode_str_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("casa['something']",False,[self.server_list[0],self.server_list[1]])
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], False, "Command execution was successful")
            self.assertEqual(command_response['traceback'].find("KeyError:")>=0, True, "Trace-back should contain KeyError")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")         

    def test_exec_undefined_target_blocking_mode_dict_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("import something; time.sleep(a+b)",True,None,{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("ImportError:")>=0, True, "Trace-back should contain ImportError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_defined_target_blocking_mode_dict_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a+c)",True,[self.server_list[0]],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_exec_multiple_target_blocking_mode_dict_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a/b)",True,[self.server_list[0],self.server_list[1]],{'a':1,'b':0})
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], False, "Command execution was successful")
            self.assertEqual(command_response['traceback'].find("ZeroDivisionError:")>=0, True, "Trace-back should contain ZeroDivisionError")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_exec_undefined_target_non_blocking_mode_dict_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a)",False,None,{'a':'something','b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("TypeError:")>=0, True, "Trace-back should contain TypeError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_defined_target_non_blocking_mode_dict_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a[3]+b)",False,[self.server_list[0]],{'a':[0,1],'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("IndexError:")>=0, True, "Trace-back should contain IndexError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_multiple_target_non_blocking_mode_dict_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a['sleeptime']+b)",False,[self.server_list[0],self.server_list[1]],{'a':{'interval':10},'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], False, "Command execution was successful")
            self.assertEqual(command_response['traceback'].find("KeyError:")>=0, True, "Trace-back should contain KeyError")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_eval_undefined_target_blocking_mode_dict_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("__import__(module)",True,None,{'module':'something','b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("ImportError:")>=0, True, "Trace-back should contain ImportError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_eval_defined_target_blocking_mode_dict_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("a+c",True,[self.server_list[0]],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_eval_multiple_target_blocking_mode_dict_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("a/b",True,[self.server_list[0],self.server_list[1]],{'a':1,'b':0})
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], False, "Command execution was successful")
            self.assertEqual(command_response['traceback'].find("ZeroDivisionError:")>=0, True, "Trace-back should contain ZeroDivisionError")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_eval_undefined_target_non_blocking_mode_dict_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("pow(a,b)",False,None,{'a':'test','b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("TypeError:")>=0, True, "Trace-back should contain TypeError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_eval_defined_target_non_blocking_mode_dict_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("a[3]+b",False,[self.server_list[0]],{'a':[0,1,2],'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("IndexError:")>=0, True, "Trace-back should contain IndexError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_eval_multiple_target_non_blocking_mode_dict_params_not_successful(self):
        
        command_request_id_list = self.client.push_command_request("a['something']+b",False,[self.server_list[0],self.server_list[1]],{'a':{'test':5},'b':2})
        
        # Try to get responses before time in non-blocking more
        command_response_list = self.client.get_command_response(command_request_id_list,False,True)
        
        # Get response in blocking mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two elements")
        for command_response in command_response_list:
            self.assertEqual(command_response['successful'], False, "Command execution was successful")
            self.assertEqual(command_response['traceback'].find("KeyError:")>=0, True, "Trace-back should contain KeyError")
            self.assertEqual(command_response['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
    def test_multiple_target_including_busy_server(self):
        
        # Send a command request to a specific server so that it is in busy state
        command_request_id_list = self.client.push_command_request("time.sleep(5); test=1",False,[self.server_list[0]])
        
        # Send a command request to a list of servers including the busy one
        command_response_list = self.client.push_command_request("test+1",True,[self.server_list[0],self.server_list[1]])
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 2, "Command response list should contain two element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 2, "Command return variable should be 2")    
        self.assertEqual(command_response_list[1]['successful'], False, "Command execution was not successful")
        self.assertEqual(command_response_list[1]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[1]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[1]['ret'], None, "Command return variable should be None")            
            
    def test_singleton_behaviour(self):
        
        # Delete current MPICommandClient singleton instance reference
        client_ref = self.client
        del client_ref
        
        # Create a new MPICommandClient singleton instance reference
        new_client_ref = MPICommandClient()
        
        # Execute some command
        command_response_list = new_client_ref.push_command_request("a+b",True,[self.server_list[0]],{'a':1,'b':1})
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 2, "Command return variable should be 2")       
           
            
class test_MPICommandServer(unittest.TestCase):            
            
    def setUp(self):
        
        self.client = MPICommandClient()
        self.client.set_log_mode('unified')
        self.server_list = MPIEnvironment.mpi_server_rank_list()
        self.client.start_services()
        
        
    def test_server_not_responsive(self):
        
        # First find a sutiable server
        rank = -1
        server_list = self.client.get_server_status()
        for server in server_list:
            if not server_list[server]['timeout']:
                 rank = server_list[server]['rank']
                 break
        
        # Overload server n# 0 with a pow operation
        command_request_id_list = self.client.push_command_request("pow(a,b)",False,[rank],{'a':10,'b':100000000000000000})
        timeout_command_id = command_request_id_list
        
        # Wait until server is not responsive
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        command_response = command_response_list[0]
        if not command_response['successful']:
            self.assertEqual(command_response['traceback'].find("Timeout")>=0, True, "Trace-back should contain Timeout")
            self.assertEqual(command_response['status'], 'timeout', "Command status should be 'timeout'")
            self.assertEqual(command_response['ret'], None, "Command return variable should be None")
            
            # Try to assign another command to the non-responsive server
            command_request_id_list = self.client.push_command_request("a+b",False,[rank],{'a':1,'b':1})
            self.assertEqual(command_request_id_list, None, "It should not be possible to push command requests to a non-responsive server")
            
        # Try to retrieve the command response in non-blocking mode with MPIInterface
        rethrow = False
        mpi_interface = MPIInterface()
        mpi_interface_core = mpi_interface._cluster
        try:
            res = mpi_interface_core.check_job(timeout_command_id,verbose=True)
        except:
            rethrow = True
            
        self.assertEqual(rethrow,True,"Exception not retrown") 
        self.assertEqual(str(sys.exc_info()[1]).find("Timeout")>=0, True, "Trace-back should contain Timeout")
            
            
    def test_server_timeout_recovery(self):
             
        # First get list of servers
        server_list = self.client.get_server_status()
        
        # Find a server deployed at localhost
        localhost_name = MPIEnvironment.hostname
        pid = -1
        rank = -1
        for server in server_list:
            if (server_list[server]['processor'] == localhost_name) and (not server_list[server]['timeout']): 
                pid = server_list[server]['pid']
                rank = server_list[server]['rank']
                break
        
        # Send stop signal to server
        if pid >= 0:
            
            # Push some command request to the server
            command_request_id_list = self.client.push_command_request("time.sleep(10)",False,[rank])
             
            # Send stop signal to server
            os.kill(pid,signal.SIGSTOP)
            
            # Wait unit the server timesout
            command_response_list = self.client.get_command_response(command_request_id_list,True,True)
            
            # Check if the server has actually timed out
            command_response = command_response_list[0]
            if not command_response['successful']:
                self.assertEqual(command_response['traceback'].find("Timeout")>=0, True, "Trace-back should contain Timeout")
                self.assertEqual(command_response['status'], 'timeout', "Command status should be 'timeout'")
                self.assertEqual(command_response['ret'], None, "Command return variable should be None")
                
            # Send continue signal to server
            os.kill(pid,signal.SIGCONT)
            
            # Wait until server has recovered
            time.sleep(2*MPIEnvironment.mpi_monitor_status_service_heartbeat)
            
            # Check if a command response has been received
            command_response_list = self.client.get_command_response(command_request_id_list,True,True)
            
            # Analyze command response list contents
            self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
            self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
            self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
            self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
            self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")              
            
                
    def test_server_cannot_be_instantiated(self):
        
        instantiated = False
        try:
            server = MPICommandServer()
            instantiated = True
        except Exception, instance:
            instantiated = False
            
        self.assertEqual(instantiated, False, "It should not be possible to instantiate MPICommandServer in the client")
        
        
    def test_server_fake_timeout_busy_wait(self):
               
        # Simulate a client timeout with a greedy operation
        nloops = len(self.server_list) / 2
        for iter in range(0,nloops):
            str(10**1000000) # NOTE: The greedy part is the str conversion
        
        # Check if any server turns into timeout condition in 2 loops of the heartbeat service
        monitorClient = MPIMonitorClient()
        end_check = time.time() + 2*MPIEnvironment.mpi_monitor_status_service_heartbeat
        while (time.time() < end_check):
            onlineServers = list(monitorClient.get_server_rank_online())
            self.assertEqual(len(self.server_list),len(onlineServers), "There are servers in timeout condition") 
        
        
class test_MPIInterface(unittest.TestCase):            
    
    def setUp(self):
        
        MPIInterface.set_log_mode('redirect')
        self.sc = MPIInterface.getCluster()
        self.CL = self.sc._cluster                  
        
    def test_PyParallelImagerHelper_interface(self):
        
        # Get cluster (getCluster should automatically initialize it)
        self.sc = MPIInterface.getCluster()
        self.CL = self.sc._cluster
        self.assertEqual(self.sc.isClusterRunning(),True,"Error instantiating cluster")

        # Get engines
        engines = self.CL.get_engines()
        self.assertEqual(engines,range(1,MPIEnvironment.mpi_world_size),"Error getting list of engines")
        
        # Get nodes
        if int(os.environ['OMPI_COMM_WORLD_LOCAL_SIZE'])>1:
            nodes = self.CL.get_nodes()
            self.assertTrue(socket.gethostname() in nodes,"Error getting list of nodes")
        
        # Run imports in all engines
        self.CL.pgc('import os')
        self.CL.pgc('from numpy import array,int32')
        os_is_module = self.CL.pgc('os is not None')[0]['ret']
        self.assertEqual(os_is_module,True,"Error importing os module")
        
        # Change current working directory
        cwd=os.getcwd()
        self.CL.pgc('os.chdir("' + cwd + '")')
        res = self.CL.pgc('os.getcwd()')[0]['ret']
        self.assertEqual(res,cwd,"Error changing work directory")
        
        # Get engine working directory
        cwd=os.getcwd()
        res = self.sc.get_engine_store(1)
        self.assertEqual(res,cwd,"Error getting engine store")
        
        # pgc/Pull variable to/from all servers
        self.CL.pgc("initrec = casac.utils().hostinfo()['endian']")
        res = self.CL.pull('initrec')
        self.assertEqual(res[1],casac.utils().hostinfo()['endian'],"Error pulling a variable")
        
        # Push/Pull variable to/from a subset of servers
        var_dict={}
        var_dict['a'] = 33
        var_dict['b'] = {'test':29.2}
        self.CL.push(var_dict,[1,2])
        res = self.CL.pull('a',[1,2])
        self.assertEqual(res[1],var_dict['a'],"Error pulling a variable after a push operation to a subset of servers")
        res = self.CL.pull('b',[1,2])
        self.assertEqual(res[2],var_dict['b'],"Error pulling a variable after a push operation to a subset of servers")      
        
        # Push/Pull variable to/from all servers
        var_dict={}
        var_dict['c'] = False
        var_dict['d'] = "bla"
        self.CL.push(var_dict)
        res = self.CL.pull('c')
        self.assertEqual(res[1],var_dict['c'],"Error pulling a variable after a push operation to all servers")
        res = self.CL.pull('d')
        self.assertEqual(res[2],var_dict['d'],"Error pulling a variable after a push operation to all servers")           
        
        # Run various commands in parallel
        self.CL.pgc({1:'ya=3',2:'ya="b"'})
        res = self.CL.pull('ya',[1,2])
        self.assertEqual(res,{1: 3, 2: 'b'},"Error running various commands in parallel")        
        
        # Async execution of a job in a subset of servers via odo
        jobIds = self.CL.odo("time.sleep(2.5)",1)
        status = self.CL.check_job(jobIds)
        ntries = 0
        while status == False and ntries < 10:
            ntries += 1
            time.sleep(1)
            status = self.CL.check_job(jobIds)        
        self.assertEqual(status,True,"Error executing a job asynchronously via odo")   
        
        # Async execution of a job in a subset of servers via do_and_record with defined target server
        jobIds = self.sc.do_and_record("time.sleep(2.5)",1)
        status = self.CL.check_job(jobIds)
        ntries = 0
        while status == False and ntries < 10:
            ntries += 1
            time.sleep(1)
            status = self.CL.check_job(jobIds)        
        self.assertEqual(status,True,"Error executing a job asynchronously via do_and_record with defined target server")   
        
        # Async execution of a job in a subset of servers via do_and_record with undefined target server
        jobIds = self.sc.do_and_record("time.sleep(2.5)")
        status = self.CL.check_job(jobIds)
        ntries = 0
        while status == False and ntries < 10:
            ntries += 1
            time.sleep(1)
            status = self.CL.check_job(jobIds)        
        self.assertEqual(status,True,"Error executing a job asynchronously via do_and_record with undefined target server")          
        
        # Re-throw exception
        jobIds = self.CL.odo("1/0",[1, 2])
        ntries = 0
        res = False
        rethrow = False
        while res == False and ntries < 10:
            try:
                res = self.CL.check_job(jobIds)
                time.sleep(1)
                ntries += 1
            except:
                rethrow = True
                break
        self.assertEqual(rethrow,True,"Exception not retrown")
        self.assertEqual(str(sys.exc_info()[1]).find("ZeroDivisionError:")>=0, True, "Trace-back should contain ZeroDivisionError")      
        
        # Check queue status
        jobIds = self.CL.odo("time.sleep(5)",1)
        time.sleep(1)
        status = self.sc.get_status()
        self.assertEqual(len(status)-1,len(self.CL.get_command_request_list()),"Error retrieving job queue status")
        
        # Stop cluster
        # status = self.sc.stop_cluster()
        # self.assertEqual(self.sc.isClusterRunning(),False,"Error stoping cluster")
      

class test_mpi4casa_flagdata(unittest.TestCase):

    def setUp(self):
        
        self.vis = "Four_ants_3C286.mms"
        setUpFile(self.vis,'vis')
        
        # Tmp files
        self.vis2 = self.vis + '.2'
        self.vis3 = self.vis + '.3'    
        
        # Set up cluster
        self.client = MPICommandClient()
        self.client.set_log_mode('redirect')
        self.client.start_services()  

    def tearDown(self):

        os.system('rm -rf ' + self.vis)
        
        # Remove tmp files
        os.system('rm -rf ' + self.vis2)
        os.system('rm -rf ' + self.vis3)        
    
    def test_mpi4casa_flagdata_list_return(self):
        """Test support for MMS using flagdata in unflag+clip mode"""

        # Create list file
        text = "mode='unflag'\n"\
               "mode='clip' clipminmax=[0,0.1]"
        filename = 'list_flagdata.txt'
        create_input(text, filename)

        # step 1: Do unflag+clip
        flagdata(vis=self.vis, mode='list', inpfile=filename)

        # step 2: Now do summary
        ret_dict = flagdata(vis=self.vis, mode='summary')

        # Check summary
        self.assertTrue(ret_dict['name']=='Summary')
        self.assertTrue(ret_dict['spw']['15']['flagged'] == 96284.0)
        self.assertTrue(ret_dict['spw']['0']['flagged'] == 129711.0)
        self.assertTrue(ret_dict['spw']['1']['flagged'] == 128551.0)
        self.assertTrue(ret_dict['spw']['2']['flagged'] == 125686.0)
        self.assertTrue(ret_dict['spw']['3']['flagged'] == 122862.0)
        self.assertTrue(ret_dict['spw']['4']['flagged'] == 109317.0)
        self.assertTrue(ret_dict['spw']['5']['flagged'] == 24481.0)
        self.assertTrue(ret_dict['spw']['6']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['7']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['8']['flagged'] == 0)
        self.assertTrue(ret_dict['spw']['9']['flagged'] == 27422.0)
        self.assertTrue(ret_dict['spw']['10']['flagged'] == 124638.0)
        self.assertTrue(ret_dict['spw']['11']['flagged'] == 137813.0)
        self.assertTrue(ret_dict['spw']['12']['flagged'] == 131896.0)
        self.assertTrue(ret_dict['spw']['13']['flagged'] == 125074.0)
        self.assertTrue(ret_dict['spw']['14']['flagged'] == 118039.0)
        
    def test_mpi4casa_flagdata_list_return_async(self):
        """Test flagdata summary in async mode"""
        
        # First run flagdata sequentially
        bypassParallelProcessing = ParallelTaskHelper.getBypassParallelProcessing()
        ParallelTaskHelper.bypassParallelProcessing(2)
        res = flagdata(vis=self.vis, mode='summary')
        ParallelTaskHelper.bypassParallelProcessing(bypassParallelProcessing)
        
        # Make a copy of the input MMS for each flagdata instance
        os.system("cp -r %s %s" % (self.vis,self.vis2))
        os.system("cp -r %s %s" % (self.vis,self.vis3))
        
        # Set async mode in ParallelTaskHelper
        ParallelTaskHelper.setAsyncMode(True)
        
        # Run applycal in MMS mode with the first set
        request_id_1 = flagdata(vis=self.vis, mode='summary')    
        
        # Run applycal in MMS mode with the second set
        request_id_2 = flagdata(vis=self.vis2, mode='summary')
        
        # Run applycal in MMS mode with the third set
        request_id_3 = flagdata(vis=self.vis3, mode='summary')
        
        # Get response in block mode
        reques_id_list = request_id_1 + request_id_2 + request_id_3
        command_response_list = self.client.get_command_response(reques_id_list,True,True)        
        
        # Get result
        res1 = ParallelTaskHelper.getResult(request_id_1,'flagdata')
        res2 = ParallelTaskHelper.getResult(request_id_2,'flagdata')
        res3 = ParallelTaskHelper.getResult(request_id_3,'flagdata')   
        
        # Unset async mode in ParallelTaskHelper
        ParallelTaskHelper.setAsyncMode(False)         
        
        self.assertEqual(res1,res, "flagdata dictionary does not match for the first flagdata run")
        self.assertEqual(res2,res, "flagdata dictionary does not match for the second flagdata run")
        self.assertEqual(res3,res, "flagdata dictionary does not match for the third flagdata run")       
        
        
    def test_mpi4casa_flagdata_list_return_multithreading(self):
        """Test flagdata summary in multithreading mode"""
        
        # First run flagdata sequentially
        bypassParallelProcessing = ParallelTaskHelper.getBypassParallelProcessing()
        ParallelTaskHelper.bypassParallelProcessing(2)
        res = flagdata(vis=self.vis, mode='summary')
        ParallelTaskHelper.bypassParallelProcessing(bypassParallelProcessing)
        
        # Make a copy of the input MMS for each flagdata instance
        os.system("cp -r %s %s" % (self.vis,self.vis2))
        os.system("cp -r %s %s" % (self.vis,self.vis3))
        
        ParallelTaskHelper.setMultithreadingMode(True)        
        
        # Set up workers
        cmd1 = "flagdata(vis='%s', mode='summary')" % (self.vis)
        worker1 = ParallelTaskWorker(cmd1)
        
        cmd2 = "flagdata(vis='%s', mode='summary')" % (self.vis2)
        worker2 = ParallelTaskWorker(cmd2)        
        
        cmd3 = "flagdata(vis='%s', mode='summary')" % (self.vis3)
        worker3 = ParallelTaskWorker(cmd3)          
        
        # Spawn worker threads
        worker1.start()
        worker2.start()
        worker3.start()
        
        # Get resulting summary ict from each worker
        res1 = worker1.getResult()
        res2 = worker2.getResult()
        res3 = worker3.getResult()
        
        ParallelTaskHelper.setMultithreadingMode(False) 
        
        # Compare return summary dicts with the one generated with a sequential run
        self.assertEqual(res1,res, "flagdata dictionary does not match for the first flagdata run")
        self.assertEqual(res2,res, "flagdata dictionary does not match for the second flagdata run")
        self.assertEqual(res3,res, "flagdata dictionary does not match for the third flagdata run")    
        
        
class test_mpi4casa_setjy(unittest.TestCase):

    def setUp(self):

        self.vis = "ngc5921.applycal.mms"
        setUpFile(self.vis,'vis')

    def tearDown(self):
        
        os.system('rm -rf ' + self.vis)

    def test1_setjy_scratchless_mode_single_model(self):
        """Test 1: Set vis model header in one single field """

        retval = setjy(vis=self.vis, field='1331+305*',fluxdensity=[1331.,0.,0.,0.], 
                       scalebychan=False, usescratch=False,standard='manual')
        self.assertTrue(retval, "setjy run failed")    
        
        mslocal = mstool()
        mslocal.open(self.vis)
        listSubMSs = mslocal.getreferencedtables()
        mslocal.close()
        #listSubMSs.append(self.vis)
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS + '/SOURCE')
            nrows = tblocal.nrows()
            for row_i in range(0,nrows):
                try:
                    model_i = tblocal.getcell('SOURCE_MODEL',row_i)
                    if (row_i == 0):
                        self.assertEqual(model_i['cl_0']['fields'][0],row_i)
                        self.assertEqual(model_i['cl_0']['container']['component0']['flux']['value'][0],1331.)
                    else:
                        self.assertEqual(len(model_i),0)
                except:
                    casalog.post("Problem accesing SOURCE_MODEL col from subMS %s" % subMS ,
                                 "SEVERE","test1_setjy_scratchless_mode_single_model")
            tblocal.close()
            
    def test2_setjy_scratchless_mode_multiple_model(self):
        """Test 2: Set vis model header in one multiple fields """

        retval = setjy(vis=self.vis, field='1331+305*',fluxdensity=[1331.,0.,0.,0.], 
                       scalebychan=False, usescratch=False,standard='manual')
        self.assertTrue(retval, "setjy run failed")
        retval = setjy(vis=self.vis, field='1445+099*',fluxdensity=[1445.,0.,0.,0.], 
                       scalebychan=False, usescratch=False,standard='manual')
        self.assertTrue(retval, "setjy run failed")
                   
        mslocal = mstool()
        mslocal.open(self.vis)
        listSubMSs = mslocal.getreferencedtables()
        mslocal.close()
        #listSubMSs.append(self.vis)
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS + '/SOURCE')
            nrows = tblocal.nrows()
            for row_i in range(0,nrows):
                try:
                    model_i = tblocal.getcell('SOURCE_MODEL',row_i)
                    if (row_i == 0):
                        self.assertEqual(model_i['cl_0']['fields'][0],row_i)
                        self.assertEqual(model_i['cl_0']['container']['component0']['flux']['value'][0],1331.)
                    elif (row_i == 1):
                        self.assertEqual(model_i['cl_0']['fields'][0],row_i)
                        self.assertEqual(model_i['cl_0']['container']['component0']['flux']['value'][0],1445.)                    
                    else:
                        self.assertEqual(len(model_i),0)
                except:
                    casalog.post("Problem accesing SOURCE_MODEL col from subMS %s" % subMS ,
                                 "SEVERE","test2_setjy_scratchless_mode_multiple_model")                        
            tblocal.close()            
            
    def test3_setjy_scratch_mode_single_model(self):
        """Test 3: Set MODEL_DATA in one single field"""

        retval = setjy(vis=self.vis, field='1331+305*',fluxdensity=[1331.,0.,0.,0.], 
                       scalebychan=False,usescratch=True,standard='manual')
        self.assertTrue(retval, "setjy run failed")
        
        mslocal = mstool()
        mslocal.open(self.vis)
        listSubMSs = mslocal.getreferencedtables()
        mslocal.close()
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS)
            fieldId = tblocal.getcell('FIELD_ID',1)
            if (fieldId == 0):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1331.0)
            elif (fieldId == 1):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1.0)
            elif (fieldId == 2):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1.0)
            else:
                raise AssertionError, "Unrecognized field [%s] found in Sub-MS [%s]" %(str(fieldId),subMS)
                tblocal.close()
            tblocal.close()

    def test4_setjy_scratch_mode_multiple_model(self):
        """Test 4: Set MODEL_DATA in multiple fields"""

        retval = setjy(vis=self.vis, field='1331+305*',fluxdensity=[1331.,0.,0.,0.], 
                       scalebychan=False, usescratch=True,standard='manual')
        self.assertTrue(retval, "setjy run failed")
        retval = setjy(vis=self.vis, field='1445+099*',fluxdensity=[1445.,0.,0.,0.], 
                       scalebychan=False, usescratch=True,standard='manual')
        self.assertTrue(retval, "setjy run failed")
        
        mslocal = mstool()
        mslocal.open(self.vis)
        listSubMSs = mslocal.getreferencedtables()
        mslocal.close()
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS)
            fieldId = tblocal.getcell('FIELD_ID',1)
            if (fieldId == 0):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1331.0)
            elif (fieldId == 1):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1445.0)
            elif (fieldId == 2):
                self.assertEqual(tblocal.getcell('MODEL_DATA',1)[0][0].real,1.0)
            else:
                raise AssertionError, "Unrecognized field [%s] found in Sub-MS [%s]" %(str(fieldId),subMS)
                tblocal.close()
            tblocal.close()        
        

class test_mpi4casa_applycal(unittest.TestCase):

    def setUp(self):
        
        # Set-up MMS
        self.vis = "ngc5921.applycal.mms"
        self.vis_sorted = "ngc5921.applycal.sorted.mms"
        setUpFile(self.vis,'vis')
        
        # Set-up reference MMS
        self.ref = "ngc5921.applycal.ms"
        self.ref_sorted = "ngc5921.applycal.sorted.ms"
        setUpFile(self.ref,'ref')
        
        # Set-up auxiliary files
        self.aux = ["ngc5921.fluxscale", "ngc5921.gcal", "ngc5921.bcal"]
        setUpFile(self.aux ,'aux')
        
        # Repository caltables are pre-v4.1, and we
        # must update them _before_ applycal to avoid contention
        casalog.post("Updating pre-v4.1 caltables: %s" % str(self.aux),"WARN","test1_applycal_fluxscale_gcal_bcal")
        cblocal = cbtool()
        for oldct in self.aux:
            cblocal.updatecaltable(oldct)
        casalog.post("Pre-v4.1 caltables updated","INFO","test_mpi4casa_applycal")        
        
        # Tmp files
        self.vis2 = self.vis + '.2'
        self.vis3 = self.vis + '.3'
        self.vis_sorted2 = self.vis_sorted + '.2'
        self.vis_sorted3 = self.vis_sorted + '.3'
        
        # Tmp aux files
        self.aux2 = []
        self.aux3 = []
        for file in self.aux:
            self.aux2.append(file + '.2')
            self.aux3.append(file + '.3')
        
        # Set up cluster
        self.client = MPICommandClient()
        self.client.set_log_mode('redirect')
        self.client.start_services()          

    def tearDown(self):
        
        # Remove MMS
        os.system('rm -rf ' + self.vis) 
        os.system('rm -rf ' + self.vis_sorted) 
        
        # Remove ref MMS
        os.system('rm -rf ' + self.ref) 
        os.system('rm -rf ' + self.ref_sorted) 
        
        # Remove tmp files
        os.system('rm -rf ' + self.vis2)
        os.system('rm -rf ' + self.vis3)
        os.system('rm -rf ' + self.vis_sorted2)
        os.system('rm -rf ' + self.vis_sorted3)        
        
        # Remove aux files
        for file in self.aux: os.system('rm -rf ' + file)
            
        # Remove tmp aux files
        for file in self.aux2: os.system('rm -rf ' + file)            
        for file in self.aux3: os.system('rm -rf ' + file)                  
                     
        
    def test1_applycal_fluxscale_gcal_bcal(self):
        """Test 1: Apply calibration using fluxscal gcal and bcal tables"""
        
        # Run applycal in MS mode
        applycal(vis=self.ref,field='',spw='',selectdata=False,gaintable=self.aux,
                 gainfield=['nearest','nearest','0'],
                 interp=['linear', 'linear','nearest'],spwmap=[])
        
        # Run applycal in MMS mode
        applycal(vis=self.vis,field='',spw='',selectdata=False,gaintable=self.aux,
                 gainfield=['nearest','nearest','0'],
                 interp=['linear', 'linear','nearest'],spwmap=[])
        
        # Sort file to properly match rows for comparison
        casalog.post("Sorting vis file: %s" % str(self.vis),"INFO","test1_applycal_fluxscale_gcal_bcal")
        sortFile(self.vis,self.vis_sorted)  
        casalog.post("Sorting ref file: %s" % str(self.ref),"INFO","test1_applycal_fluxscale_gcal_bcal")    
        sortFile(self.ref,self.ref_sorted)        
        
        # Compare files
        compare = testhelper.compTables(self.ref_sorted,self.vis_sorted,['FLAG_CATEGORY'])
        self.assertTrue(compare)      
        
    def test2_applycal_fluxscale_gcal_bcal_async_mode(self):
        """Test 2: Apply calibration using fluxscal gcal and bcal tables in async mode"""
        
        # Run applycal in MS mode
        applycal(vis=self.ref,gaintable=self.aux,
                 gainfield=['nearest','nearest','0'],
                 interp=['linear', 'linear','nearest'])        
       
        # Make a copy of the input MMS for each applycal instance
        os.system("cp -r %s %s" % (self.vis,self.vis2))
        os.system("cp -r %s %s" % (self.vis,self.vis3))
        
        # Make a copy of cal tables for each applycal instance
        for idx in range(0,len(self.aux)):
             os.system("cp -r %s %s" % (self.aux[idx],self.aux2[idx]))
             os.system("cp -r %s %s" % (self.aux[idx],self.aux3[idx]))
             
        # Set async mode in ParallelTaskHelper
        ParallelTaskHelper.setAsyncMode(True)
        
        # Run applycal in MMS mode with the first set
        request_id_1 = applycal(vis=self.vis,gaintable=self.aux,
                                gainfield=['nearest','nearest','0'],
                                interp=['linear', 'linear','nearest'])    
        
        # Run applycal in MMS mode with the second set
        request_id_2 = applycal(vis=self.vis2,gaintable=self.aux2,
                                gainfield=['nearest','nearest','0'],
                                interp=['linear', 'linear','nearest'])    
        
        # Run applycal in MMS mode with the third set
        request_id_3 = applycal(vis=self.vis3,gaintable=self.aux3,
                                gainfield=['nearest','nearest','0'],
                                interp=['linear', 'linear','nearest'])   
        
        # Get response in block mode
        reques_id_list = request_id_1 + request_id_2 + request_id_3
        command_response_list = self.client.get_command_response(reques_id_list,True,True)        
        
        # Unset async mode in ParallelTaskHelper
        ParallelTaskHelper.setAsyncMode(False)
        
        # Sort ref file to properly match rows for comparison
        casalog.post("Sorting vis file: %s" % str(self.vis),"INFO","test2_applycal_fluxscale_gcal_bcal_async_mode")
        sortFile(self.vis,self.vis_sorted)  
        casalog.post("Sorting vis file: %s" % str(self.vis2),"INFO","test2_applycal_fluxscale_gcal_bcal_async_mode")
        sortFile(self.vis2,self.vis_sorted2)       
        casalog.post("Sorting vis file: %s" % str(self.vis3),"INFO","test2_applycal_fluxscale_gcal_bcal_async_mode")
        sortFile(self.vis3,self.vis_sorted3)              
        casalog.post("Sorting ref file: %s" % str(self.ref),"INFO","test2_applycal_fluxscale_gcal_bcal_async_mode")    
        sortFile(self.ref,self.ref_sorted)        
        
        # Compare files
        compare = testhelper.compTables(self.ref_sorted,self.vis_sorted,['FLAG_CATEGORY'])
        self.assertTrue(compare)
        compare = testhelper.compTables(self.ref_sorted,self.vis_sorted2,['FLAG_CATEGORY'])
        self.assertTrue(compare)
        compare = testhelper.compTables(self.ref_sorted,self.vis_sorted3,['FLAG_CATEGORY'])  
        
        
    def test3_applycal_fluxscale_gcal_bcal_multithreading_mode(self):
        """Test 2: Apply calibration using fluxscal gcal and bcal tables in multithreading mode"""
        
        # Run applycal in MS mode
        applycal(vis=self.ref,gaintable=self.aux,
                 gainfield=['nearest','nearest','0'],
                 interp=['linear', 'linear','nearest'])        
       
        # Make a copy of the input MMS for each applycal instance
        os.system("cp -r %s %s" % (self.vis,self.vis2))
        os.system("cp -r %s %s" % (self.vis,self.vis3))
        
        # Make a copy of cal tables for each applycal instance
        for idx in range(0,len(self.aux)):
             os.system("cp -r %s %s" % (self.aux[idx],self.aux2[idx]))
             os.system("cp -r %s %s" % (self.aux[idx],self.aux3[idx]))
        
        ParallelTaskHelper.setMultithreadingMode(True)
        
        cmd1 = "applycal(vis='%s',gaintable=['%s','%s','%s'],gainfield=['nearest','nearest','0'],interp=['linear', 'linear','nearest'])" % (self.vis,self.aux[0],self.aux[1],self.aux[2])
        worker1 = ParallelTaskWorker(cmd1)
        
        cmd2 = "applycal(vis='%s',gaintable=['%s','%s','%s'],gainfield=['nearest','nearest','0'],interp=['linear', 'linear','nearest'])" % (self.vis2,self.aux2[0],self.aux2[1],self.aux2[2])
        worker2 = ParallelTaskWorker(cmd2)        
        
        cmd3 = "applycal(vis='%s',gaintable=['%s','%s','%s'],gainfield=['nearest','nearest','0'],interp=['linear', 'linear','nearest'])" % (self.vis3,self.aux3[0],self.aux3[1],self.aux3[2])
        worker3 = ParallelTaskWorker(cmd3)          
        
        worker1.start()
        worker2.start()
        worker3.start()
        
        worker1.getResult()
        worker2.getResult()
        worker3.getResult()
        
        ParallelTaskHelper.setMultithreadingMode(False)
        
        # Sort ref file to properly match rows for comparison
        casalog.post("Sorting vis file: %s" % str(self.vis),"INFO","test2_applycal_fluxscale_gcal_bcal_async_mode")
        sortFile(self.vis,self.vis_sorted)  
        casalog.post("Sorting vis file: %s" % str(self.vis2),"INFO","test2_applycal_fluxscale_gcal_bcal_async_mode")
        sortFile(self.vis2,self.vis_sorted2)       
        casalog.post("Sorting vis file: %s" % str(self.vis3),"INFO","test2_applycal_fluxscale_gcal_bcal_async_mode")
        sortFile(self.vis3,self.vis_sorted3)              
        casalog.post("Sorting ref file: %s" % str(self.ref),"INFO","test2_applycal_fluxscale_gcal_bcal_async_mode")    
        sortFile(self.ref,self.ref_sorted)        
        
        # Compare files
        compare = testhelper.compTables(self.ref_sorted,self.vis_sorted,['FLAG_CATEGORY'])
        self.assertTrue(compare)
        compare = testhelper.compTables(self.ref_sorted,self.vis_sorted2,['FLAG_CATEGORY'])
        self.assertTrue(compare)
        compare = testhelper.compTables(self.ref_sorted,self.vis_sorted3,['FLAG_CATEGORY'])          

        
class test_mpi4casa_uvcont(unittest.TestCase):

    def setUp(self):           
        # Set-up MMS
        self.vis = "ngc5921.uvcont.mms"
        setUpFile(self.vis,'vis')
        # Tmp files
        self.vis2 = self.vis + '.2'
        self.vis3 = self.vis + '.3'        
        # Set-up reference MMS
        self.ref = ["ngc5921.mms.cont", "ngc5921.mms.contsub"]
        setUpFile(self.ref,'ref')      

    def tearDown(self):
        # Remove MMS
        os.system('rm -rf ' + self.vis + '*')
        # Remove tmp files
        os.system('rm -rf ' + self.vis2 + '*')
        os.system('rm -rf ' + self.vis3 + '*')        
        # Remove ref MMS
        for file in self.ref:
            os.system('rm -rf ' + file) 
        
    def test1_uvcont_single_spw(self):
        """Test 1: Extract continuum from one single SPW using uvcontsub"""

        uvcontsub(vis=self.vis,field = 'N5921*',fitspw='0:4~6;50~59',spw = '0',
                  solint = 'int',fitorder = 0,want_cont = True) 
        
        compare_cont = testhelper.compTables(self.ref[0],self.vis+".cont",['FLAG_CATEGORY','WEIGHT','SIGMA'])
        self.assertTrue(compare_cont)
        compare_contsub = testhelper.compTables(self.ref[1],self.vis+".contsub",['FLAG_CATEGORY','WEIGHT','SIGMA'])
        self.assertTrue(compare_contsub)             
        
    def test2_uvcont_single_spw_multithreading_mode(self):
        """Test 2: Extract continuum from one single SPW using uvcontsub in multithreading mode"""   
                
        # First run in sequential mode
        os.system("cp -r %s %s" % (self.vis,self.vis2))
        os.system("cp -r %s %s" % (self.vis,self.vis3))
        
        ParallelTaskHelper.setMultithreadingMode(True)

        cmd1 = "uvcontsub(vis='%s',field = 'N5921*',fitspw='0:4~6;50~59',spw = '0',solint = 'int',fitorder = 0,want_cont = True)" % (self.vis)
        worker1 = ParallelTaskWorker(cmd1)
        
        cmd2 = "uvcontsub(vis='%s',field = 'N5921*',fitspw='0:4~6;50~59',spw = '0',solint = 'int',fitorder = 0,want_cont = True)" % (self.vis2)
        worker2 = ParallelTaskWorker(cmd2)        
        
        cmd3 = "uvcontsub(vis='%s',field = 'N5921*',fitspw='0:4~6;50~59',spw = '0',solint = 'int',fitorder = 0,want_cont = True)" % (self.vis3)
        worker3 = ParallelTaskWorker(cmd3)          
        
        worker1.start()
        worker2.start()
        worker3.start()
        
        worker1.getResult()
        worker2.getResult()
        worker3.getResult()
        
        ParallelTaskHelper.setMultithreadingMode(False)        
        
        compare_cont = testhelper.compTables(self.ref[0],self.vis+".cont",['FLAG_CATEGORY','WEIGHT','SIGMA'])
        self.assertTrue(compare_cont)
        compare_contsub = testhelper.compTables(self.ref[1],self.vis+".contsub",['FLAG_CATEGORY','WEIGHT','SIGMA'])
        self.assertTrue(compare_contsub)   
        
        compare_cont2 = testhelper.compTables(self.ref[0],self.vis2+".cont",['FLAG_CATEGORY','WEIGHT','SIGMA'])
        self.assertTrue(compare_cont2)
        compare_contsub2 = testhelper.compTables(self.ref[1],self.vis2+".contsub",['FLAG_CATEGORY','WEIGHT','SIGMA'])
        self.assertTrue(compare_contsub2)   
        
        compare_cont3 = testhelper.compTables(self.ref[0],self.vis3+".cont",['FLAG_CATEGORY','WEIGHT','SIGMA'])
        self.assertTrue(compare_cont3)
        compare_contsub3 = testhelper.compTables(self.ref[1],self.vis3+".contsub",['FLAG_CATEGORY','WEIGHT','SIGMA'])
        self.assertTrue(compare_contsub3)
        
        
class test_mpi4casa_NullSelection(unittest.TestCase):

    def setUp(self):
        
        self.vis = "Four_ants_3C286.mms"
        setUpFile(self.vis,'vis')
        
        self.client = MPICommandClient()
        self.client.set_log_mode('redirect')
        self.client.start_services()       
        
        # Prepare list of servers
        self.server_list = []
        server_list = self.client.get_server_status()
        for server in server_list:
            if not server_list[server]['timeout']:
                self.server_list.append(server_list[server]['rank'])          
                
        self.client.push_command_request("import os",True,self.server_list)

    def tearDown(self):

        os.system('rm -rf ' + self.vis)
        
        # Restore log file and filter
        self.client.push_command_request("casalog.setlogfile(casa['files']['logfile'])",True,self.server_list)        
    
    def test_mpi4casa_NullSelection_entire_mms(self):
        """Test filter out NullSelection exceptions"""
        
        # First clear list of filter out msgs. and make sure that the MSSelectionNullSelection shows up
        for server in self.server_list:
            logfile = 'MSSelectionNullSelection-Not-Filtered.log-server-%s' % str(server)
            self.client.push_command_request("casalog.setlogfile('%s'); casalog.clearFilterMsgList()" % (logfile),True,server)
            
        # Run flagdata selecting a non-existing scan
        flagdata(vis=self.vis, scan='99')  
        
        # Iterate trough log files to see if we find the exception
        for server in self.server_list:
            # Get current working directory (we might be in the 'nosedir' subdirectory)
            cwd = self.client.push_command_request("os.getcwd()",True,server)[0]['ret']
            logfile = '%s/MSSelectionNullSelection-Not-Filtered.log-server-%s' % (cwd,str(server))
            content = open(logfile, 'r').read()
            if content.find('flagdata')>0: # Check only server with processed a flagdata sub-job
                self.assertEqual(content.find("MSSelectionNullSelection")>0, True, "MSSelectionNullSelection should not be filtered out")

        # Now populate the list of msg to be filter out including MSSelectionNullSelection
        text = ['MSSelectionNullSelection','NeverHappens']
        for server in self.server_list:
            logfile = 'MSSelectionNullSelection-Filtered.log-server-%s' % str(server)
            self.client.push_command_request("casalog.setlogfile('%s'); casalog.filterMsg(%s)" % (logfile,str(text)),True,server) 
        
        # Run flagdata selecting a non-existing scan
        flagdata(vis=self.vis, scan='99')  
        
        # Iterate trough log files to see if we find the exception
        for server in self.server_list:
            # Get current working directory (we might be in the 'nosedir' subdirectory)
            cwd = self.client.push_command_request("os.getcwd()",True,server)[0]['ret']
            logfile = '%s/MSSelectionNullSelection-Filtered.log-server-%s' % (cwd,str(server))
            content = open(logfile, 'r').read()
            if content.find('flagdata')>0: # Check only server with processed a flagdata sub-job
                self.assertEqual(content.find("MSSelectionNullSelection")<0, True, "MSSelectionNullSelection should be filtered out")       
            

class test_mpi4casa_log_level(unittest.TestCase):

    def setUp(self):
        
        self.vis = "Four_ants_3C286.mms"
        setUpFile(self.vis,'vis')
        
        self.client = MPICommandClient()
        self.client.set_log_mode('redirect')
        self.client.start_services()       
        
        # Prepare list of servers
        self.server_list = []
        server_list = self.client.get_server_status()
        for server in server_list:
            if not server_list[server]['timeout']:
                self.server_list.append(server_list[server]['rank'])          
                
        self.client.push_command_request("import os",True,self.server_list)

    def tearDown(self):

        os.system('rm -rf ' + self.vis)
        
        # Restore log file and level
        self.client.push_command_request("casalog.setlogfile(casa['files']['logfile'])",True,self.server_list)
        self.client.set_log_level("INFO")
    
    def test_mpi4casa_log_level_default_to_debug(self):
        """Test changing globally log level from default to debug """
            
        # Change log level globally (test via MPIInterface as it internally uses MPICommandClient so both are tested)
        mpi_interface = MPIInterface()
        mpi_interface.set_log_level("DEBUG")    
                
        # Use a separated log file per server to facilitate analysis
        for server in self.server_list:
            logfile = 'test_mpi4casa_log_level_debug-server-%s.log' % str(server)
            self.client.push_command_request("casalog.setlogfile('%s')" % (logfile),True,server)        
            
        # Run flagdata 
        flagdata(vis=self.vis, mode='summary')  
        
        # Iterate trough log files to see if we find command handling msgs
        for server in self.server_list:
            # Get current working directory (we might be in the 'nosedir' subdirectory)
            cwd = self.client.push_command_request("os.getcwd()",True,server)[0]['ret']
            logfile = '%s/test_mpi4casa_log_level_debug-server-%s.log' % (cwd,str(server))
            content = open(logfile, 'r').read()
            if content.find('flagdata')>0: # Check only server with processed a flagdata sub-job
                self.assertEqual(content.find("MPICommandServer")<0, True, "MPICommandServer msgs should be filtered out")                 

                
                
class test_mpi4casa_plotms(unittest.TestCase):

    def setUp(self):
        
        self.vis = 'Four_ants_3C286.mms'
        setUpFile(self.vis,'vis')
        
        self.client = MPICommandClient()
        self.client.set_log_mode('redirect')
        self.client.start_services()       
        
        # Prepare list of servers
        self.server_list = []
        server_list = self.client.get_server_status()
        for server in server_list:
            if not server_list[server]['timeout']:
                self.server_list.append(server_list[server]['rank'])          

    def tearDown(self):

        os.system('rm -rf ' + self.vis)
    
    def test_mpi4casa_plotms_concurrent(self):
        """Run plotms on the same MS from each server simulateneously"""
        
        # Change current working directory
        self.client.push_command_request("os.chdir('%s')" % os.getcwd(),True,self.server_list)
        
        # Farm plotms jobs
        command_request_id_list = []
        for server in self.server_list:
            plotfile = 'test_mpi4casa_plotms_concurrent-%s.png' % str(server)
            cmd = "plotms('%s', avgchannel='8',avgtime='60s',plotfile='%s',showgui=False)" % (self.vis,plotfile)
            command_request_id = self.client.push_command_request(cmd,False,server)
            command_request_id_list.append(command_request_id[0])
            
        # Get response in block mode
        command_response_list = self.client.get_command_response(command_request_id_list,True,True)
        
        # Compare files
        for server_idx in range(0,len(self.server_list)):
            for server_idy in range(server_idx+1,len(self.server_list)):
                server_x = self.server_list[server_idx]
                server_y = self.server_list[server_idy]
                plotfile_server_idx = 'test_mpi4casa_plotms_concurrent-%s.png' % str(server_x)
                plotfile_server_idy = 'test_mpi4casa_plotms_concurrent-%s.png' % str(server_y)
                areEqual = filecmp.cmp(plotfile_server_idx,plotfile_server_idy)
                self.assertTrue(areEqual,"Plotfile generated by server %s is different from plotfile generated by server %s" 
                                % (str(server_x),str(server_y)))
        

def suite():
    return [test_MPICommandClient,
            test_MPIInterface,
            test_mpi4casa_flagdata,
            test_mpi4casa_setjy,
            test_mpi4casa_applycal,
            test_mpi4casa_uvcont,
            test_MPICommandServer,
            test_mpi4casa_NullSelection,
            test_mpi4casa_plotms,
            test_mpi4casa_log_level]
