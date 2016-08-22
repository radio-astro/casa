c = get_config()  # get the Config instance

c.TerminalIPythonApp.display_banner = False

c.InteractiveShellApp.ignore_old_config=True 
c.InteractiveShellApp.exec_lines=[ 'from asap import *', 'print welcome()' ]
#c.InteractiveShellApp.exec_files=[ '~/.asap/asapuserfuncs.py' ]

c.InteractiveShell.autoindent = True
c.InteractiveShell.autocall = 2
c.InteractiveShell.magic_pprint = True
c.InteractiveShell.use_readline=True
c.InteractiveShell.confirm_exit=False
c.InteractiveShell.xmode='Plain'

c.PromptManager.in_template='ASAP> '
c.PromptManager.out_template='asap>: '

