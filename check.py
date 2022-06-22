
""" # noqa
   ___           __________________  ___________
  / _/__  ____  / __/ ___/  _/ __/ |/ / ___/ __/
 / _/ _ \/ __/ _\ \/ /___/ // _//    / /__/ _/          # noqa
/_/ \___/_/   /___/\___/___/___/_/|_/\___/___/          # noqa
Author : Benjamin Blundell - k1803390@kcl.ac.uk

check.py - check that the data is being worked out right

"""


import csv
import numpy as np
import argparse
import os
from astropy.io import fits
from scipy.cluster.vq import kmeans
from vedo import Points, show, Volume

if __name__ == "__main__":
    # Training settings
    parser = argparse.ArgumentParser(description="Wiggle Three Check")

    parser.add_argument(
        "--image", default="./test.obj", help="The path to the image we are checking."
    )
    parser.add_argument(
        "--log", default="./log.csv", help="The path to the csv data log file."
    )

    parser.add_argument(
        "--width",
        type=int,
        default=128,
        help="The width of the input and output images \
                          (default: 128).",
    )
    parser.add_argument(
        "--height",
        type=int,
        default=128,
        help="The height of the input and output images \
                          (default: 128).",
    )
    parser.add_argument(
        "--depth",
        type=int,
        default=16,
        help="The depth of the input and output images \
                          (default: 16).",
    )
    parser.add_argument(
        "--sigma",
        type=float,
        default=1.0,
        help="The sigma to render for prediction (default 1.0)",
    )
    parser.add_argument(
        "--max-trans",
        type=float,
        default=0.5,
        help="The maximum translation amount (default 0.5)",
    )
 
    args = parser.parse_args()

    points = []

    if os.path.exists(args.log):
        with open(args.log) as csvfile:
            reader = csv.DictReader(csvfile, delimiter=',')
            for row in reader:
                if row['id'] in args.image: # Cheap and nasty check really :/
                    p0 = [float(row['p0x']), float(row['p0y']), float(row['p0z'])]
                    points.append(p0)
                    p1 = [float(row['p1x']), float(row['p1y']), float(row['p1z'])]
                    points.append(p1)
                    p2 = [float(row['p2x']), float(row['p2y']), float(row['p2z'])]
                    points.append(p2)
                    p3 = [float(row['p3x']), float(row['p3y']), float(row['p3z'])]
                    points.append(p3)
              
    else:
        print("log.csv must exist along with the images.")
        assert(False)
    
    with fits.open(args.image) as w:
        #hdul = w[0].data.astype('float32')
        hdul = w[0].data.byteswap().newbyteorder().astype('float32')
        target = np.array(hdul, dtype=np.float32)
        target_vol = target * 255

        # Interpolate the depths
        # https://stackoverflow.com/questions/28934767/best-way-to-interpolate-a-numpy-ndarray-along-an-axis
        from scipy.interpolate import interp1d
        depths = np.linspace(0, 1, args.depth)
        f_out = interp1d(depths, target_vol, axis=0)
        new_depths = np.linspace(0, 1, args.width)
        target_vol = f_out(new_depths)
        target_vol = target_vol.astype(np.uint8)

        scalar = args.width / args.depth
    
        for i in range(len(points)):
            points[i][2] = points[i][2] * scalar
      
        vedo_vol = Volume(target_vol, alpha=0.4, mode=1)
        vedo_points = Points(points, r=12).c('red')
        show(vedo_vol, vedo_points, __doc__, axes=1, viewup='y').close()