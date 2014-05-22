import os
import time
import signal
import unittest
import testhelper
from taskinit import mstool,tbtool,cbtool,casalog
from tasks import setjy,flagdata,applycal,uvcontsub
from mpi4casa.MPIEnvironment import MPIEnvironment
from mpi4casa.MPICommandClient import MPICommandClient
from mpi4casa.MPICommandServer import MPICommandServer
from parallel.parallel_task_helper import ParallelTaskHelper


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
    os.system('cp -R ' + os.environ.get('CASAPATH').split()[0] + 
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
    
    client = MPICommandClient()
    client.start_services()
            
    def test_exec_undefined_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(3)",True,None)
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")
        
    def test_exec_defined_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(3)",True,[0])
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_exec_multiple_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(3)",True,[0,1])
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
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(3)",False,[0])
        
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
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(3)",False,[0,1])
        
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
        
        command_response_list = self.client.push_command_request("1+1",True,[0])
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 2, "Command return variable should be 2")  
        
    def test_eval_multiple_target_blocking_mode_str_params_successful(self):
        
        command_response_list = self.client.push_command_request("1+1",True,[0,1])
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
        
        command_request_id_list = self.client.push_command_request("1+1",False,[0])
        
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
        
        command_request_id_list = self.client.push_command_request("1+1",False,[0,1])
        
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
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a+b)",True,[0],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_exec_multiple_target_blocking_mode_dict_params_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a+b)",True,[0,1],{'a':1,'b':2})
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
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a+b)",False,[0],{'a':1,'b':2})
        
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
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a+b)",False,[0,1],{'a':1,'b':2})
        
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
        
        command_response_list = self.client.push_command_request("a+b",True,[0],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 3, "Command return variable should be 3")  
        
    def test_eval_multiple_target_blocking_mode_dict_params_successful(self):
        
        command_response_list = self.client.push_command_request("a+b",True,[0,1],{'a':1,'b':2})
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
        
        command_request_id_list = self.client.push_command_request("a+b",False,[0],{'a':1,'b':2})
        
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
        
        command_request_id_list = self.client.push_command_request("a+b",False,[0,1],{'a':1,'b':2})
        
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
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a)",True,[0])
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_exec_multiple_target_blocking_mode_str_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(3/0)",True,[0,1])
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
        
        command_request_id_list = self.client.push_command_request("test=[];test[1]=2",False,[0])
        
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
        
        command_request_id_list = self.client.push_command_request("test={};test['bananas']",False,[0,1])
        
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
        
        command_response_list = self.client.push_command_request("a+1",True,[0])
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_eval_multiple_target_blocking_mode_str_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("1/0",True,[0,1])
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
        
        command_request_id_list = self.client.push_command_request("test=[0,1]",True,[0])
        command_request_id_list = self.client.push_command_request("test[3]",False,[0])
        
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
        
        command_request_id_list = self.client.push_command_request("casa['something']",False,[0,1])
        
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
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a+c)",True,[0],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_exec_multiple_target_blocking_mode_dict_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("import time; time.sleep(a/b)",True,[0,1],{'a':1,'b':0})
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
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a[3]+b)",False,[0],{'a':[0,1],'b':2})
        
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
        
        command_request_id_list = self.client.push_command_request("import time; time.sleep(a['sleeptime']+b)",False,[0,1],{'a':{'interval':10},'b':2})
        
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
        
        command_response_list = self.client.push_command_request("a+c",True,[0],{'a':1,'b':2})
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], False, "Command execution was successful")
        self.assertEqual(command_response_list[0]['traceback'].find("NameError:")>=0, True, "Trace-back should contain NameError")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], None, "Command return variable should be None")  
        
    def test_eval_multiple_target_blocking_mode_dict_params_not_successful(self):
        
        command_response_list = self.client.push_command_request("a/b",True,[0,1],{'a':1,'b':0})
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
        
        command_request_id_list = self.client.push_command_request("a[3]+b",False,[0],{'a':[0,1,2],'b':2})
        
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
        
        command_request_id_list = self.client.push_command_request("a['something']+b",False,[0,1],{'a':{'test':5},'b':2})
        
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
        command_request_id_list = self.client.push_command_request("time.sleep(5); test=1",False,[0])
        
        # Send a command request to a list of servers including the busy one
        command_response_list = self.client.push_command_request("test+1",True,[0,1])
        
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
        command_response_list = new_client_ref.push_command_request("a+b",True,[0],{'a':1,'b':1})
        
        # Analyze command response list contents
        self.assertEqual(len(command_response_list), 1, "Command response list should contain one element")
        self.assertEqual(command_response_list[0]['successful'], True, "Command execution was not successful")
        self.assertEqual(command_response_list[0]['traceback'], None, "Command execution trace-back should be None")
        self.assertEqual(command_response_list[0]['status'], 'response received', "Command status should be 'response received'")
        self.assertEqual(command_response_list[0]['ret'], 2, "Command return variable should be 2")       
           
            
class test_MPICommandServer(unittest.TestCase):            
            
    client = MPICommandClient()
    client.start_services()
        
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
        

class test_mpi4casa_flagdata(unittest.TestCase):

    def setUp(self):
        
        self.vis = "Four_ants_3C286.mms"
        setUpFile(self.vis,'vis')

    def tearDown(self):

        os.system('rm -rf ' + self.vis)
    
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
        listSubMSs.append(self.vis)
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
        listSubMSs.append(self.vis)
        for subMS in listSubMSs:
            tblocal = tbtool()
            tblocal.open(subMS + '/SOURCE')
            nrows = tblocal.nrows()
            for row_i in range(0,nrows):
                model_i = tblocal.getcell('SOURCE_MODEL',row_i)
                if (row_i == 0):
                    self.assertEqual(model_i['cl_0']['fields'][0],row_i)
                    self.assertEqual(model_i['cl_0']['container']['component0']['flux']['value'][0],1331.)
                elif (row_i == 1):
                    self.assertEqual(model_i['cl_0']['fields'][0],row_i)
                    self.assertEqual(model_i['cl_0']['container']['component0']['flux']['value'][0],1445.)                    
                else:
                    self.assertEqual(len(model_i),0)
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

    def tearDown(self):
        # Remove MMS
        os.system('rm -rf ' + self.vis) 
        os.system('rm -rf ' + self.vis_sorted) 
        # Remove ref MMS
        os.system('rm -rf ' + self.ref) 
        os.system('rm -rf ' + self.ref_sorted) 
        # Remove aux files
        for file in self.aux:
            os.system('rm -rf ' + file)         
        
    def test1_applycal_fluxscale_gcal_bcal(self):
        """Test 1: Apply calibration using fluxscal gcal and bcal tables"""

        # Repository caltables are pre-v4.1, and we
        # must update them _before_ applycal to avoid contention
        casalog.post("Updating pre-v4.1 caltables: %s" % str(self.aux),"WARN","test1_applycal_fluxscale_gcal_bcal")
        cblocal = cbtool()
        for oldct in self.aux:
            cblocal.updatecaltable(oldct)
        casalog.post("Pre-v4.1 caltables updated","INFO","test1_applycal_fluxscale_gcal_bcal")
        
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

        
class test_mpi4casa_uvcont(unittest.TestCase):

    def setUp(self):           
        # Set-up MMS
        self.vis = "ngc5921.uvcont.mms"
        setUpFile(self.vis,'vis')
        # Set-up reference MMS
        self.ref = ["ngc5921.mms.cont", "ngc5921.mms.contsub"]
        setUpFile(self.ref,'ref')      

    def tearDown(self):
        # Remove MMS
        os.system('rm -rf ' + self.vis)
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
        

def suite():
    return [test_MPICommandClient,
            test_MPICommandServer,
            test_mpi4casa_flagdata,
            test_mpi4casa_setjy,
            test_mpi4casa_applycal,
            test_mpi4casa_uvcont]
