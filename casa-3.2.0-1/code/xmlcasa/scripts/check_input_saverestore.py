tasklist = ['accum','bandpass','blcal','browsetable','clean','clearcal','concat','contsub','correct','exportuvfits','feather','flagautocorr','flagdata','flagxy','fluxscale','fringecal','ft','gaincal','imhead','immoments','importvla','importasdm','importgmrt','importuvfits','imsmooth','listhistory','listcal','listobs','listvis','makemask','mosaic','plotants','plotcal','plotms','plotxy','pointcal','polcal','setjy','split','uvmodelfit','viewer', 'widefield','msmoments']

for task in tasklist:
	print 'task is ',task
	inp(task)
	saveinputs(task)
	taskparameters=task+'.saved'
	execfile(taskparameters)
