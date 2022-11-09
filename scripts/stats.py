
""" # noqa
   ___           __________________  ___________
  / _/__  ____  / __/ ___/  _/ __/ |/ / ___/ __/
 / _/ _ \/ __/ _\ \/ /___/ // _//    / /__/ _/          # noqa
/_/ \___/_/   /___/\___/___/___/_/|_/\___/___/          # noqa
Author : Benjamin Blundell - k1803390@kcl.ac.uk

stats.py - look at the worm data and generate some stats

Example use:
python stats.py --base /phd/wormz/queelim --rep /media/proto_backup/wormz/queelim

"""

from statistics import median
import torch
import numpy as np
import argparse
import pickle
import os
from tqdm import tqdm
import torch.nn as nn
from matplotlib import pyplot as plt
from scipy.cluster.vq import kmeans
from astropy.io import fits
from util.math import PointsTen, gen_scale, gen_trans, Point, Points
from datafiles import data_files


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

def stats_from_data(args):
    '''Derive the stats from the included dat files. '''

    asi_1_total = []
    asi_2_total = []
    asj_1_total = []
    asj_2_total = []

    backgrounds = []

    for _, anno_dir in data_files:
        tdir = anno_dir.replace(args.base, args.rep)

        print(tdir)

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

                            t0 = lines[0].split(" ")
            
                            backgrounds.append(int(t0[7]))
                
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

    print ("Counts", len(asi_1_total),  len(asi_2_total), len(asj_1_total), len(asj_2_total))

    asi_1_normed = (asi_1_total - np.min(asi_1_total)) / (np.max(asi_1_total) - np.min(asi_1_total))
    asi_2_normed = (asi_2_total - np.min(asi_2_total)) / (np.max(asi_2_total) - np.min(asi_2_total))
    asj_1_normed = (asj_1_total - np.min(asj_1_total)) / (np.max(asj_1_total) - np.min(asj_1_total))
    asj_2_normed = (asj_2_total - np.min(asj_2_total)) / (np.max(asj_2_total) - np.min(asj_2_total))

    print("Mean Scores ASI-1, ASI-2, ASJ-1, ASJ-2", np.mean(asi_1_normed), np.mean(asi_2_normed), np.mean(asj_1_normed), np.mean(asj_2_normed))
    print("Std Dev ASI-1, ASI-2, ASJ-1, ASJ-2", np.std(asi_1_normed), np.std(asi_2_normed), np.std(asj_1_normed), np.std(asj_2_normed))

    backgrounds = np.array(backgrounds)

    print("Background counts Mean, Median, Std", np.mean(backgrounds), np.median(backgrounds), np.std(backgrounds))

    #plt.bar(asi_1_hist[1][:-1], asi_1_hist[0], width=0.1)
    #plt.show()

    #plt.hist(hists, 10, histtype='step', stacked=True, fill=False, label=labels)

    ncols = 2
    nrows = 2
    fig, axes = plt.subplots(nrows=nrows, ncols=ncols, figsize=(10, 10))

    ax = axes[0][0]
    ax.hist(asi_1_normed, bins=100, alpha=0.5, label="ASI-1")
    ax.set_xlabel('Normalised Intensity')
    ax.set_ylabel('Count')
    ax.legend(prop={'size': 10})
    ax = axes[0][1]
    ax.hist(asi_2_normed, bins=100, alpha=0.5, label="ASI-2")
    ax.set_xlabel('Normalised Intensity')
    ax.set_ylabel('Count')
    ax.legend(prop={'size': 10})
    ax = axes[1][0]
    ax.hist(asj_1_normed, bins=100, alpha=0.5, label="ASJ-1")
    ax.set_xlabel('Normalised Intensity')
    ax.set_ylabel('Count')
    ax.legend(prop={'size': 10})
    ax = axes[1][1]
    ax.hist(asj_2_normed, bins=100, alpha=0.5, label="ASJ-2")
    ax.set_xlabel('Normalised Intensity')
    ax.set_ylabel('Count')
    ax.legend(prop={'size': 10})
    plt.show()

    backgrounds = np.array(backgrounds)

    print("Background counts Mean, Median, Std", np.mean(backgrounds), np.median(backgrounds), np.std(backgrounds))

    ncols = 2
    nrows = 2
    fig, axes = plt.subplots(nrows=nrows, ncols=ncols, figsize=(10, 10))

    ax = axes[0][0]
    ax.hist(asi_1_total, bins=100, alpha=0.5, label="ASI-1")
    ax.set_xlabel('Intensity')
    ax.set_ylabel('Count')
    ax.legend(prop={'size': 10})
    ax = axes[0][1]
    ax.hist(asi_2_total, bins=100, alpha=0.5, label="ASI-2")
    ax.set_xlabel('Intensity')
    ax.set_ylabel('Count')
    ax.legend(prop={'size': 10})
    ax = axes[1][0]
    ax.hist(asj_1_total, bins=100, alpha=0.5, label="ASJ-1")
    ax.set_xlabel('Intensity')
    ax.set_ylabel('Count')
    ax.legend(prop={'size': 10})
    ax = axes[1][1]
    ax.hist(asj_2_total, bins=100, alpha=0.5, label="ASJ-2")
    ax.set_xlabel('Intensity')
    ax.set_ylabel('Count')
    ax.legend(prop={'size': 10})
    plt.show()

def stats_from_raw(args):
    import math

    data = {}

    if args.load != "" and os.path.exists(args.load):
        with open(args.load, 'rb') as f:
            data = pickle.load(f)
    else:
        data = read_from_raw(args)

    import matplotlib.pyplot as plt
    import seaborn as sns
    import pandas as pd
  
    # Multiclass alignments
    sns.set_theme(style="darkgrid")
    data_frame = pd.DataFrame(data["per_image_hists"]).transpose()
    print(data_frame)
    image_idx = 11

    # The range of intensity values we want to plot for
    rg = (300,3826)
    #sns.lineplot(data=data_frame[ranges[0]:ranges[1]], x=data_frame.index[ranges[0]:ranges[1]], y=data_frame.iloc[:, 11][ranges[0]:ranges[1]])

    val, weight = zip(*[(k + rg[0], v) for k, v in enumerate(list(data["hist"])[rg[0]:rg[1]]) ])
    fig, ax = plt.subplots()
    ax.hist(val, weights=weight, bins=20)
    ax.set_xlabel('Intensity Value')
    ax.set_ylabel('Count')
    ax.set_title("Histogram of intensity values (" + str(rg[0]) + " to " + str(rg[1]) + ") across entire C.elegans dataset")
    plt.show()

    val, weight = zip(*[(k + rg[0], v) for k, v in enumerate(data["per_image_hists"][image_idx][rg[0]:rg[1]])])
    plt.hist(val, weights=weight, bins=100)
    plt.show()

    min_score = 4096
    max_score = 0
    biggest_score = 0
    biggest_cuml = 0
    median_cuml = 0
    median_score = 0
    mean_cuml = 0
    score_std = 0
    mean_score = 0
    all_scores = data["hist"]
    total = np.sum(list(all_scores))
    print("Total scores", total)

    for hidx in range(0, 4095):
        score_count = all_scores[hidx]

        if score_count > biggest_cuml:
            biggest_score = hidx
            biggest_cuml = score_count

        if hidx >= max_score and score_count > 0:
            max_score = hidx

        if hidx <= min_score and score_count > 0:
            min_score = hidx

        mean_score += score_count
        mean_cuml += hidx * score_count

    mean_score = mean_cuml / mean_score

    for hidx in range(0, 4095):
        score_count = all_scores[hidx]
        median_cuml += score_count
        
        if median_cuml >= int(total / 2):
            median_score = hidx
            break

    std_cuml = 0

    for hidx in range(0, 4095):
        score_count = all_scores[hidx]
        dd = math.pow(hidx - mean_score, 2) * score_count  
        std_cuml += dd

    score_std = math.sqrt(std_cuml / total)

    print("All Scores Min, Max, Mean, median, std", min_score, max_score, mean_score, median_score, score_std)
    print("Intensity with the largest score", biggest_score)

    modes = []
    for idx, mode in enumerate(data["per_image_modes"]):
        if all(x < 500 for x in mode):
            modes = modes + mode
        else:
            filename = data["filenames"][idx]
            print("Error in file", filename, mode)

    print("Mode scores min, max, std", np.min(modes), np.max(modes), np.std(modes))

    # Percentiles by count
    histo = data["per_image_hists"][0]
    
    for hist in data["per_image_hists"][1:]:
        histo = histo + hist

    num_pix_p = np.sum(histo) * 0.95
    r = 0
    cdx = 0
    
    for idx, i in enumerate(histo):
        cdx = idx
        r += i
        if r >= num_pix_p:
            break

    print("95th percentile Intensity value", cdx)

    r = 0
    cdx = 0
    
    for idx, i in enumerate(histo):
        if idx == 270:
            break
        cdx = idx
        r += i
    
    print("Percentile at 270", r / np.sum(histo) * 100.0)

    # Percentiles by total light
    total_intensity = np.sum( [idx * i for idx, i in enumerate(histo) ] ) 
    num_pix_p = total_intensity * 0.95

    r = 0
    cdx = 0
    
    for idx, i in enumerate(histo):
        cdx = idx
        r += (i * idx)
        if r >= num_pix_p:
            break

    print("95th percentile of integrated Intensity value", cdx)

    r = 0
    cdx = 0
    
    for idx, i in enumerate(histo):
        if idx == 270:
            break
        cdx = idx
        r += (i * idx)
    
    print("Integrated Intensity Percentile at 270", r / total_intensity * 100.0)

    '''
    fig, axes = plt.subplots(2, 2)

    # Joined alignments
    sns.set_theme(style="darkgrid")
    fig, axes = plt.subplots(1, 2)

    individuals = list(range(len(data["asi_1_actual"])))
    df0 = pd.DataFrame({"individual": individuals, "asi_combo_real": asi_combo_real, "asi_combo_pred": asi_combo_pred})
    df1 = pd.DataFrame({"individual": individuals, "asj_combo_real": asj_combo_real, "asj_combo_pred": asj_combo_pred})
   
    axes[0].xaxis.set_label_text("individuals")
    axes[1].xaxis.set_label_text("individuals")
   
    axes[0].yaxis.set_label_text("luminance")
    axes[1].yaxis.set_label_text("luminance")

    sns.lineplot(x="individual", y='value', hue='variable', 
             data=pd.melt(df0, ['individual']), ax=axes[0])
  
    sns.lineplot(x="individual", y='value', hue='variable', 
             data=pd.melt(df1, ['individual']), ax=axes[1])
    '''
    

def read_from_raw(args):
    ''' Generate stats from the raw data itself.'''

    intensities = []
    savedfilenames = []
    per_image_hist = []
    histo = [0 for i in range(4096)]
    per_image_modes = []

    pbar = tqdm(total=len(data_files))
    mini = 4096
    maxi = 0

    print("Reading images.")

    for image_dir, data_dir in data_files:
        new_base = image_dir.replace(args.base, args.rep)

        for dirname, dirnames, filenames in os.walk(new_base):
        
            for i in range(len(filenames)):
                filename = filenames[i]
                img_extentions = ["fits", "FITS"]

                if any(x in filename for x in img_extentions):
                    # We need to check there are no duffers in this list
                    path = os.path.join(dirname, filename)
                    savedfilenames.append(path)

                    with fits.open(path) as hdul:
                        data = np.array(hdul[0].data, dtype=np.int32)
                        vals, counts = np.unique(data, return_counts=True)
                        tt = np.argwhere(counts == np.max(counts)).flatten().tolist()
                        modes = []

                        for t in tt:
                            modes.append(vals[tt])
                      
                        intensity = np.sum(data)
                        intensities.append(intensity)

                        tlist = data.flatten()
                        hist, _ = np.histogram(tlist, bins=4096, range=(0,4095))
                    
                        per_image_hist.append(hist)
                        per_image_modes.append(modes)

                        for i in range(4096):
                            histo[i] += hist[i]

                        if np.min(data) < mini:
                            mini = np.min(data)
                        if np.max(data) > maxi:
                            maxi = np.max(data)

        pbar.update(1)
    pbar.close()

    data = {}
    data["intensities"] = intensities
    data["filenames"] = savedfilenames
    data["per_image_hists"] = per_image_hist
    data["per_image_modes"] = per_image_modes
    data["hist"] = histo

    with open('source_images_stats.pickle', 'wb') as f:
        # Pickle the 'data' dictionary using the highest protocol available.
        pickle.dump(data, f, pickle.HIGHEST_PROTOCOL)

    return data


if __name__ == "__main__":
    # Training settings
    parser = argparse.ArgumentParser(description="Wiggle Data Analysis")

    parser.add_argument('--use-dat', action='store_true', default=False,
                    help='Use the dat files instead of images')

    parser.add_argument('--base', default="/phd/wormz/queelim")
    parser.add_argument('--rep', default="/media/proto_backup/wormz/queelim")
    parser.add_argument('--load', default="")

    args = parser.parse_args()
    
    if args.use_dat:
        stats_from_data(args)
    else:
        stats_from_raw(args)
  