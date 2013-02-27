import numpy

import pipeline.infrastructure.api as api


class BPOrder(api.Heuristic):
    def calculate(self, channel_data={'DV01':[]}):
        """Method to estimate the order of polynomial required to fit the
        give data sequence. Does this by:
        
        # Fourier transform the sequence
        # Find highest frequency in the amplitude spectrum > 10 * mean
        # Order = max(2, 3 * frequency)
        
        Keyword arguments:
        data -- A dictionary of data sequences keyed by antenna identifier
        
        Returns: order - order of polynomial required.
        """
        # a list that will hold all our estimated polynomial orders for the
        # antenna
        poly_orders = []

        # we ignore the antenna identifier in our heuristic. Perhaps you might
        # want to weight the polynomials differently based on this id.        
        for antenna, data in channel_data.items():
            transform = numpy.abs(numpy.fft.fft(data)) 
            ntransform = numpy.array(transform)
            ntransform /= numpy.mean(ntransform) 
            
            if max(ntransform) < 3.0:
                poly_orders.append(1)
            elif max(ntransform) < 10.0:
                poly_orders.append(2)
            else:
                for i in range(int(len(ntransform)/2.0), -1, -1):
                    if ntransform[i] > 10.0:
                        break 
                order = max(2, 3*i)
                poly_orders.append(order)
                        
        return max(poly_orders)
