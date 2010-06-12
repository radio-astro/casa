from asapconfig import *

# the name of the observatory
observatory['name'] = 'Mopra'

# Append observing directories
observatory['rpfpath'].append("/DATA/MPCCC1_1/corr/dat/")

# Restfrequencies in GHz
observatory['lines'] = {
    'SiO (85640.456 MHz)': 85.640456,
    'SiO (86243.442 MHz)': 86.243442,
    'H13CO+ (86754.294 MHz)' : 86754.294,
    'SiO (86846.998 MHz)' : 86.846998,
    'HCN (88631.847 MHz)' : 88.631847,
    'HCO+ (89188.518 MHz)' : 89.188518,
    'HNC (90663.574 MHz)' : 90.663574,
    'N2H+ (93173.809 MHz)' : 93.173809,
    'CS (97980.968 MHz)' : 97.980968,
    '12C18O(1-0) (109782.182 MHz)' : 109.782182,
    '13CO(1-0) (110201.37 MHz)' : 110.20137,
    '12CO(1-0) (115271.203 MHz)' : 115.271203
}
