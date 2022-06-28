
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
from matplotlib import pyplot as plt
from scipy.cluster.vq import kmeans
from holly.math import PointsTen, gen_scale, gen_trans, Point, Points

data_files = [ 
["/phd/wormz/queelim/ins-6-mCherry/20170724-QL285_S1-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL285_S1-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170724-QL604_S1-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL604_S1-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170724-QL922_S1-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL922_S1-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170724-QL923_S1-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL923_S1-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170724-QL925_S1-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170724-QL925_S1-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170803-QL285_SB1-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL285_SB1-d1.0"], 
["/phd/wormz/queelim/ins-6-mCherry/20170803-QL285_SB2-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL285_SB2-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170803-QL604_SB2-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL604_SB2-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170803-QL922_SB2-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL922_SB2-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170803-QL923_SB2-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation//20170803-QL923_SB2-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170803-QL925_SB2-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170803-QL925_SB2-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170804-QL285_SB3-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL285_SB3-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170804-QL604_SB3-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL604_SB3-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170804-QL922_SB3-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL922_SB3-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170804-QL923_SB3-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL923_SB3-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry/20170804-QL925_SB3-d1.0", "/phd/wormz/queelim/ins-6-mCherry/Annotation/20170804-QL925_SB3-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170810-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170810-QL603-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL603-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170810-QL806-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL806-d1.0"], 
["/phd/wormz/queelim/ins-6-mCherry_2/20170810-QL867-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170810-QL867-d1.0"], 
["/phd/wormz/queelim/ins-6-mCherry_2/20170817-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170817-QL603-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL603-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170817-QL806-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL806-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170817-QL867-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170817-QL867-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170818-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170818-QL603-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL603-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170818-QL806-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL806-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170818-QL867-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Hai_analysis/20170818-QL867-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170821-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170821-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170821-QL569-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170821-QL569-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170825-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170825-QL569-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL569-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170825-QL849-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL849-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170828-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170828-QL569-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL569-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170828-QL849-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL849-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL568-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL568-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL823-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL823-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL824-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL824-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL835-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170907-QL835-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL568-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL568-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL823-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL823-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL824-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL824-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL835-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170908-QL835-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL568-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL568-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL823-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL823-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL824-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL824-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL835-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH\ ARZ\ analysis/20170911-QL835-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180126-QL285-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL285-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180126-QL569-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL569-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180126-QL849-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180126-QL849-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180201-QL285-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL285-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180201-QL569-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL569-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180201-QL849-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180201-QL849-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180202-QL285-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL285-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180202-QL417-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL417-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180202-QL787-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL787-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180202-QL795-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180202-QL795-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180208-QL285-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180208-QL285-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180302-QL285-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL285-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180302-QL285-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL285-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180302-QL849-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180302-QL849-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180308-QL285-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL285-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180308-QL569-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL569-d0.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20180308-QL849-d0.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20180308-QL849-d0.0"]
]

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
    
    parser.add_argument('--base', default="/phd/wormz/queelim")
    args = parser.parse_args()
    
    asi_1_total = []
    asi_2_total = []
    asj_1_total = []
    asj_2_total = []

    for image_dir, data_dir in data_files:
        tdir = data_dir.replace("phd/wormz/queelim", args.base)

        for dirname, dirnames, filenames in os.walk(tdir):
            for filename in filenames:
                if ".dat" in filename and "dist" not in filename:
                    with open(tdir + "/" + filename,'r') as f:
                        lines = f.readlines()
                        if len(lines) == 4:
                            asi_1 = lines[0].replace("[", "").replace("]","").split(" ")
                            asi_2 = lines[1].replace("[", "").replace("]","").split(" ")
                            asj_1 = lines[2].replace("[", "").replace("]","").split(" ")
                            asj_2 = lines[3].replace("[", "").replace("]","").split(" ")

                            asi_1_total.append(float(asi_1[1]))
                            asi_2_total.append(float(asi_2[1]))
                            asj_1_total.append(float(asj_1[1]))
                            asj_2_total.append(float(asj_2[1]))

                        else:
                            print(tdir + "/" + filename, "num lines", len(lines))

    asi_1_total = np.array(asi_1_total)
    asi_2_total = np.array(asi_2_total)
    asj_1_total = np.array(asj_1_total)
    asj_2_total = np.array(asj_2_total)

    asi_1_normed = (asi_1_total - np.min(asi_1_total)) / (np.max(asi_1_total) - np.min(asi_1_total))
    asi_2_normed = (asi_2_total - np.min(asi_2_total)) / (np.max(asi_2_total) - np.min(asi_2_total))
    asj_1_normed = (asj_1_total - np.min(asj_1_total)) / (np.max(asj_1_total) - np.min(asj_1_total))
    asj_2_normed = (asj_2_total - np.min(asj_2_total)) / (np.max(asj_2_total) - np.min(asj_2_total))

    print("Mean Scores ASI-1, ASI-2, ASJ-1, ASJ-2", np.mean(asi_1_normed), np.mean(asi_2_normed), np.mean(asj_1_normed), np.mean(asj_2_normed))
    print("Std Dev ASI-1, ASI-2, ASJ-1, ASJ-2", np.std(asi_1_normed), np.std(asi_2_normed), np.std(asj_1_normed), np.std(asj_2_normed))

    asi_1_hist = np.histogram(asi_1_normed)
    asi_2_hist = np.histogram(asi_2_normed)
    asj_1_hist = np.histogram(asj_1_normed)
    asj_2_hist = np.histogram(asj_2_normed)

    print(asi_1_hist)

    hists = np.stack((asi_1_normed, asi_2_normed, asj_1_normed, asj_2_normed), axis = 1)
    labels = ["ASI-1", "ASI-2", "ASJ-1", "ASJ-2"]

    # plt.bar(asi_1_hist[1][:-1], asi_1_hist[0], width=0.1)
    # plt.show()

    plt.hist(hists, 10, histtype='step', stacked=True, fill=False, label=labels)
    plt.legend(prop={'size': 10})
    plt.show()