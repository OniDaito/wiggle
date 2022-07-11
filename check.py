
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
import scipy
from astropy.io import fits
from scipy.cluster.vq import kmeans
from vedo import Points, show, Volume
from holly.plyobj import save_obj

if __name__ == "__main__":
    # Training settings
    parser = argparse.ArgumentParser(description="Wiggle Three Check")

    parser.add_argument(
        "--image", default="./test.fits", help="The path to the image we are checking."
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
        hdul = w[0].data.byteswap().newbyteorder().astype('int32')
        target = np.array(hdul, dtype=np.int32)
        target_vol = target * 255

        # get the 4 different masks
        target_unique = np.unique(target, return_index=False, return_inverse=False, return_counts=False, axis=None)
        volumes = []
        neuron_points = []

        print("Uniques", target_unique)

        for idu in target_unique:
            # ignore the 0 first one
            if idu != 0:
                tvol = np.where(target == idu, 1, 0)
                volumes.append(tvol)
                indices = np.where(target == idu) 
                coordinates = np.array(list(zip(indices[2], indices[1], (indices[0] / args.depth) * args.width)))
                neuron_points.append(coordinates)

        # OBJ points
        obj_points = np.vstack(neuron_points)
        obj_points = (obj_points / args.width) * 2.0 - 1.0
        save_obj("worm.obj", obj_points)
        

        # Now find the hulls
        hulls = []

        for npoints in neuron_points:
            hull = scipy.spatial.ConvexHull(npoints)
            fhull = []

            for vidx in hull.vertices:
                fhull.append(npoints[vidx])

            hulls.append(np.array(fhull))

        # Interpolate the depths
        # https://stackoverflow.com/questions/28934767/best-way-to-interpolate-a-numpy-ndarray-along-an-axis
        from scipy.interpolate import interp1d
        depths = np.linspace(0, 1, args.depth)
        f_out = interp1d(depths, target_vol, axis=0)
        new_depths = np.linspace(0, 1, args.width)
        target_vol = f_out(new_depths)
        target_vol = target_vol.astype(np.uint8)

        for i in range(len(points)):
            #points[i][0] = points[i][0]
            #points[i][1] = points[i][1]
            points[i][2] = (points[i][2] / args.depth) * args.width

        vedo_vol = Volume(target_vol, alpha=0.4, mode=1)
        vedo_points = Points(points, r=12).c('red')

        # First two hulls
        hull_0 = Points(neuron_points[0], r=6).c('green')
        hull_1 = Points(neuron_points[1], r=6).c('green')

        show(vedo_vol, vedo_points, hull_0, hull_1,  __doc__, axes=1, viewup='y').close()    