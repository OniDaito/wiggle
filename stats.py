
""" # noqa
   ___           __________________  ___________
  / _/__  ____  / __/ ___/  _/ __/ |/ / ___/ __/
 / _/ _ \/ __/ _\ \/ /___/ // _//    / /__/ _/          # noqa
/_/ \___/_/   /___/\___/___/___/_/|_/\___/___/          # noqa
Author : Benjamin Blundell - k1803390@kcl.ac.uk

stats.py - look at the worm data and generate some stats

"""

import torch
import numpy as np
import argparse
import os
import torch.nn as nn
import csv
from scipy.cluster.vq import kmeans
from vedo import Points as VedoPoints
from vedo import show
from holly.math import PointsTen, gen_scale, gen_trans, Point, Points

def reverse_graph(graph: Points, image_size):
    ''' The graph from the imageload .csv file comes in 3D image co-ordinates so it needs to be altered to fit.
    We are performing almost the reverse of the renderer.'''
    # For now, stick with a cube of -1 to 1 on each dimension
    assert(image_size[0] == image_size[1])
    dim = image_size[0]
    tgraph = PointsTen()
    tgraph.from_points(graph)
    scale = 2.0 / dim
    scale_mat = gen_scale(torch.tensor([scale]), torch.tensor([scale]), torch.tensor([scale]))
    trans_mat = gen_trans(torch.tensor([-1.0]), torch.tensor([-1.0]), torch.tensor([-1.0]))
    tgraph.data = torch.matmul(scale_mat, tgraph.data)
    tgraph.data = torch.matmul(trans_mat, tgraph.data)
    ngraph = tgraph.get_points()
    return ngraph


if __name__ == "__main__":
    # Training settings
    parser = argparse.ArgumentParser(description="NOPE Analysis")

    parser.add_argument(
        "--data", default="./images", help="The path to the images we used on this network."
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

    args = parser.parse_args()
    graphs = []
    neuron_positions = []
    image_size = (args.width, args.height, args.depth)

    if os.path.exists(args.data + "/log.csv"):
        with open(args.data + "/log.csv") as csvfile:
            reader = csv.DictReader(csvfile, delimiter=',')
            for row in reader:
                points = Points()
                p0 = Point(float(row['p0x']), float(row['p0y']), float(row['p0z']), 1.0)
                points.append(p0)
                p1 = Point(float(row['p1x']), float(row['p1y']), float(row['p1z']), 1.0)
                points.append(p1)
                p2 = Point(float(row['p2x']), float(row['p2y']), float(row['p2z']), 1.0)
                points.append(p2)
                p3 = Point(float(row['p3x']), float(row['p3y']), float(row['p3z']), 1.0)
                points.append(p3)
                graphs.append(points)
    else:
        print("log.csv must exist along with the images.")
        assert(False)

    for graph in graphs:
        reversed = reverse_graph(graph, image_size)
        neurons = ( (reversed.data[0].x, reversed.data[0].y, reversed.data[0].z),
            (reversed.data[1].x, reversed.data[1].y, reversed.data[1].z),
            (reversed.data[2].x, reversed.data[2].y, reversed.data[2].z),
            (reversed.data[3].x, reversed.data[3].y, reversed.data[3].z)
        )

        neuron_positions.append(neurons)
    
    neuron_positions = np.array(neuron_positions)
    asi = neuron_positions[:,:2,:]
    asj = neuron_positions[:,2:,:]
  
    from scipy.spatial import distance
    from scipy.stats import spearmanr
      
    # ASI
    distances_asi = []

    for i in range(asi.shape[0]):
        asi_1 = asi[i,0,:].squeeze()
        asi_2 = asi[i,1,:].squeeze()
        distances_asi.append(distance.euclidean(asi_1, asi_2))

    print("ASI Mean / std / min / max dist", np.mean(distances_asi), np.std(distances_asi), np.min(distances_asi), np.max(distances_asi))
    
    # ASJ
    distances_asj = []

    for i in range(asj.shape[0]):
        asj_1 = asj[i,0,:].squeeze()
        asj_2 = asj[i,1,:].squeeze()
        distances_asj.append(distance.euclidean(asj_1, asj_2))

    print("ASJ Mean / std / min / max dist", np.mean(distances_asj), np.std(distances_asj), np.min(distances_asj), np.max(distances_asj))

    # Skew between ASI/ASJ
    skews = []
    
    for i in range(asj.shape[0]):
        skews.append(np.absolute(distances_asi[i] - distances_asj[i]))
    
    print("Skews ASI - ASJ mean / std / min / max ", np.mean(skews), np.std(skews), np.min(skews), np.max(skews))
    print(spearmanr(distances_asi, distances_asj))

    # Dist between pairs
    distances = []
    asi_pair = []
    asj_pair = []

    for i in range(asj.shape[0]):
        asi_1 = asi[i,0,:].squeeze()
        asi_2 = asi[i,1,:].squeeze()
        asi_pair.append((asi_1 + asi_2) / 2)
        asj_1 = asj[i,0,:].squeeze()
        asj_2 = asj[i,1,:].squeeze()
        asj_pair.append((asj_1 + asj_2) / 2)

    asi_pair = np.array(asi_pair)
    asj_pair = np.array(asj_pair)

    for i in range(asj.shape[0]):
        asiv = asi_pair[i,:].squeeze()
        asjv = asj_pair[i,:].squeeze()
        distances.append(distance.euclidean(asiv, asjv))

    print("ASI to ASJ Mean / std / min / max dist", np.mean(distances), np.std(distances), np.min(distances), np.max(distances))

    # Skew between 1/2
    skews = []
    distances_1 = []
    distances_2 = []

    for i in range(asi.shape[0]):
        asi_1 = asi[i,0,:].squeeze()
        asi_2 = asi[i,1,:].squeeze()
        asj_1 = asj[i,0,:].squeeze()
        asj_2 = asj[i,1,:].squeeze()
        distances_1.append(distance.euclidean(asi_1, asj_1))
        distances_2.append(distance.euclidean(asi_2, asj_2))

    for i in range(asj.shape[0]):
        skews.append(np.absolute(distances_1[i] - distances_2[i]))
    
    print("Skews 1 - 2 mean / std / min / max ", np.mean(skews), np.std(skews), np.min(skews), np.max(skews))
    print(spearmanr(distances_1, distances_2))

    # Plainarity
    planes = []

    for i in range(asi.shape[0]):
        p0 = neuron_positions[i,0,:].squeeze()
        p1 = neuron_positions[i,1,:].squeeze()
        p2 = neuron_positions[i,2,:].squeeze()
        p3 = neuron_positions[i,3,:].squeeze()
        v0 = p1 - p0
        v1 = p3 - p0
        v2 = p0 - p3
        v3 = p2 - p3
        n0 = np.cross(v0, v1)
        n1 = np.cross(v2, v3)
        angle = np.dot(n0, n1)
        planes.append(angle)
    
    print("Planarity 1 - 2 mean / std / min / max ", np.mean(planes), np.std(planes), np.min(planes), np.max(planes))
