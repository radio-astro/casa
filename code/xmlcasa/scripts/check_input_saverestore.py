tasklist = ['accum','bandpass','blcal','browsetable','clean','clearcal','concat','contsub','correct','exportuvfits','feather','flagautocorr','flagdata','flagxy','fluxscale','fringecal','ft','gaincal','imhead','immoments','importvla','importasdm','importuvfits','invert','listhistory','listcal','listobs','listvis','makemask','mosaic','plotants','plotcal','plotxy','pointcal','polcal','setjy','split','uvmodelfit','viewer']

for task in tasklist:
	print 'task is ',task
	inp(task)
	saveinputs(task)
	taskparameters=task+'.saved'
	execfile(taskparameters)
