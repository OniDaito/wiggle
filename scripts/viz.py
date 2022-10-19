""" # noqa
   ___           __________________  ___________
  / _/__  ____  / __/ ___/  _/ __/ |/ / ___/ __/
 / _/ _ \/ __/ _\ \/ /___/ // _//    / /__/ _/          # noqa
/_/ \___/_/   /___/\___/___/___/_/|_/\___/___/          # noqa
Author : Benjamin Blundell - k1803390@kcl.ac.uk

viz.py - check that the data is being worked out right

"""


import csv
import numpy as np
import argparse
import os
import scipy
from astropy.io import fits
from scipy.cluster.vq import kmeans
from vedo import Points, show, Volume
from util.plyobj import save_obj

if __name__ == "__main__":
    # Training settings
    parser = argparse.ArgumentParser(description="Wiggle Three Check")

    parser.add_argument(
        "--image", default="./test.fits", help="The path to the image we are checking."
    )
   
    args = parser.parse_args()
    
    with fits.open(args.image) as w:
        #hdul = w[0].data.astype('float32')
        hdul = w[0].data.byteswap().newbyteorder().astype('float')
        target = np.array(hdul, dtype=np.float32)
 
        print(target)
        vedo_vol = Volume(target, alpha=0.4, mode=1)
      
        show(vedo_vol,  __doc__, axes=1, viewup='y').close()    