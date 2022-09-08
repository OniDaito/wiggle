
""" # noqa
   ___           __________________  ___________
  / _/__  ____  / __/ ___/  _/ __/ |/ / ___/ __/
 / _/ _ \/ __/ _\ \/ /___/ // _//    / /__/ _/          # noqa
/_/ \___/_/   /___/\___/___/___/_/|_/\___/___/          # noqa
Author : Benjamin Blundell - k1803390@kcl.ac.uk

stats.py - look at the worm data and generate some stats

Example use:
python unet_stats.py --base /media/proto_backup/wormz/queelim --dataset /media/proto_backup/wormz/queelim/dataset_24_09_2021 --savedir /media/proto_working/runs/wormz_2022_09_02

"""

import pickle
import torch
import numpy as np
import argparse
import csv
import os
import sys
import torch.nn as nn
from matplotlib import pyplot as plt
from scipy.cluster.vq import kmeans
from util.loadsave_unet import load_model, load_checkpoint
from util.math import PointsTen, gen_scale, gen_trans, Point, Points
from util.image_unet import load_fits, reduce_result, save_image, resize_3d
import torch.nn.functional as F


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
["/phd/wormz/queelim/ins-6-mCherry_2/20170821-QL849-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170821-QL849-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170825-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170825-QL569-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL569-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170825-QL849-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170825-QL849-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170828-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170828-QL569-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL569-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170828-QL849-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/Reesha_analysis/20170828-QL849-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170907-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL568-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170907-QL568-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL823-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170907-QL823-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL824-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170907-QL824-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170907-QL835-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170907-QL835-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170908-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL568-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170908-QL568-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL823-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170908-QL823-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL824-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170908-QL824-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170908-QL835-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170908-QL835-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL285-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170911-QL285-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL568-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170911-QL568-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL823-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170911-QL823-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL824-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170911-QL824-d1.0"],
["/phd/wormz/queelim/ins-6-mCherry_2/20170911-QL835-d1.0", "/phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170911-QL835-d1.0"],
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

def gen_counts(args, nclasses):
    pairs = {}

    if os.path.exists(args.dataset + "/dataset.log"):
        with open(args.dataset + "/dataset.log") as f:
            for line in f.readlines():
                if "Renaming" in line and "_WS2" in line:
                    tokens = line.split(" ")

                    if len(tokens) == 4:
                        original = tokens[1]
                        idx = int(tokens[3].replace("\n",""))
                        pairs[idx] = original
                    else:
                        # Probably spaces in the path
                        original = (" ".join(tokens[1:-2])).replace("\n","")
                        idx = int(tokens[-1].replace("\n",""))
                        pairs[idx] = original
    
    # Find the test set for a particular run
    dataset = []
    final_sources = []
    final_masks = []

    if os.path.exists(args.savedir + "/dataset_test.csv"):
        with open(args.savedir + "/dataset_test.csv") as csvfile:
            reader = csv.DictReader(csvfile, delimiter=',')
            
            for row in reader:
                idx = int(row['source'].split("_")[0])
                dataset.append(idx)
                path = args.dataset + "/" + row['source']
                final_sources.append(path)
                path = args.dataset + "/" + row[' target'].replace(" ", "")
                final_masks.append(path)

    # Now look for the written down values in the data files
    # Find the final filenames but remove the tiff
    prefix_idx = []

    for idx in dataset:
        path = pairs[idx]
        head, tail = os.path.split(path)
        head, pref = os.path.split(head)
        _, pref2 = os.path.split(head)
        final = pref2 + "/" + pref + "/" + tail
        final = final.replace("tiff", "")
        final = final.replace("_WS2","")
        prefix_idx.append((final, idx))

    # We now have the input tiff file. We need to find it's corresponding annotation file.

    test_set_files = []
    asi_1_total = []
    asi_2_total = []
    asj_1_total = []
    asj_2_total = []
    file_lookups = []

    for image_dir, data_dir in data_files:
        tdir = data_dir.replace("phd/wormz/queelim", args.base)

        for dirname, dirnames, filenames in os.walk(tdir):

            for filename in filenames:

                if ".dat" in filename and "_2" in filename and "ID" in filename: 
                    tfile = tdir + "/" + filename
                    head, tail = os.path.split(tfile)
                    head, pref = os.path.split(head)
                    _, pref2 = os.path.split(head)
                    ft = pref2 + "/" + pref + "/" + tail
                    ft = ft.replace("dat", "")
                    ft = ft.replace("_2","")
                    file_lookups.append((ft, tfile))


    #print(file_lookups)

    # Now match input tiff and id to the dat file
    for fname, fidx in prefix_idx:

        search = [i for i in file_lookups if i[0] == fname]
        print(search)

        if len(search) != 0:
            fpath = pairs[fidx]
            test_set_files.append(fpath)

            with open(search[0][1],'r') as f:
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
                    print("Error in reading dat file.")
                    print(tdir + "/" + filename, "num lines", len(lines))
                    sys.exit(1)
        
        else:
            print("Failed to find", fname)
            sys.exit(1)


    # We should now have the neuron counts and the list of files in test_set_files
    assert(len(dataset) == len(asi_1_total))

    asi_1_total_pred = []
    asi_2_total_pred = []
    asj_1_total_pred = []
    asj_2_total_pred = []

    # Now load the model to test it's predictions
    use_cuda = not args.no_cuda and torch.cuda.is_available()
    device = torch.device("cuda" if use_cuda else "cpu")

    if args.savedir and os.path.isfile(args.savedir + "/checkpoint.pth.tar"):
        # (savedir, savename) = os.path.split(args.load)
        # print(savedir, savename)
        model = load_model(args.savedir + "/model.tar")
        (model, _, _, _, _, prev_args) = load_checkpoint(
            model, args.savedir, "checkpoint.pth.tar", device
        )
        model = model.to(device)
        model.eval()

        for fidx, path in enumerate(final_sources):
            print("Testing", path)
            input_image = load_fits(path, dtype=torch.float32)
            resized_image = resize_3d(input_image, 0.5)
            normalised_image = resized_image / 4095.0

            with torch.no_grad():
                im = normalised_image.unsqueeze(dim=0).unsqueeze(dim=0)
                im = im.to(device)
                prediction = model.forward(im)

                #with open('prediction.npy', 'wb') as f:
                #   np.save(f, prediction.detach().cpu().numpy())
                
                assert(not (torch.all(prediction == 0).item()))

                classes = prediction[0].detach().cpu().squeeze()
                classes = F.one_hot(classes.argmax(dim=0), nclasses).permute(3, 0, 1, 2)
                classes = np.argmax(classes, axis=0)
                
                resized_prediction = classes.detach()
                resized_prediction = resize_3d(resized_prediction, 2.0)
                resized_prediction = torch.narrow(resized_prediction, 0, 0, input_image.shape[0])
                print("Shapes", classes.shape, resized_prediction.shape, input_image.shape)
                assert(resized_prediction.shape[2] == input_image.shape[2])
     
                final = reduce_result(prediction)
                save_image(final, name="guess" + str(fidx) + ".jpg")
           
                count_asi_1 = 0
                count_asi_2 = 0
                count_asj_1 = 0
                count_asj_2 = 0
              
                print("Classes", resized_prediction.shape)

                asi_1_mask = torch.where(resized_prediction == 1, 1, 0)
                asi_2_mask = torch.where(resized_prediction == 2, 1, 0)
                asj_1_mask = torch.where(resized_prediction == 3, 1, 0)
                asj_2_mask = torch.where(resized_prediction == 4, 1, 0)

                count_asi_1 = float(torch.sum(asi_1_mask * input_image))
                count_asi_2 = float(torch.sum(asi_2_mask * input_image))
                count_asj_1 = float(torch.sum(asj_1_mask * input_image))
                count_asj_2 = float(torch.sum(asj_2_mask * input_image))

                print("Counts", count_asi_1, count_asi_2, count_asj_1, count_asj_2)

                asi_1_total_pred.append(count_asi_1)
                asi_2_total_pred.append(count_asi_2)
                asj_1_total_pred.append(count_asj_1)
                asj_2_total_pred.append(count_asj_2)

    data = {}
    data["asi_1_actual"] = asi_1_total
    data["asi_2_actual"] = asi_2_total
    data["asj_1_actual"] = asj_1_total
    data["asj_2_actual"] = asj_2_total
    data["asi_1_pred"] = asi_1_total_pred
    data["asi_2_pred"] = asi_2_total_pred
    data["asj_1_pred"] = asj_1_total_pred
    data["asj_2_pred"] = asj_2_total_pred
    data["source_files"] = test_set_files
    data["prediction_files"] = test_set_files

    with open('data.pickle', 'wb') as f:
        # Pickle the 'data' dictionary using the highest protocol available.
        pickle.dump(data, f, pickle.HIGHEST_PROTOCOL)

    return data


if __name__ == "__main__":
    # Training settings
    parser = argparse.ArgumentParser(description="U-Net Data Analysis")

    parser.add_argument('--dataset', default="/phd/wormz/queelim/dataset_24_09_2021")
    parser.add_argument('--savedir', default=".")
    parser.add_argument('--base', default="/phd/wormz/queelim")
    parser.add_argument(
        "--no-cuda", action="store_true", default=False, help="disables CUDA training"
    )
    parser.add_argument('--load', default="")
    args = parser.parse_args()
    nclasses = 5
    data = None

    if args.load != "" and os.path.exists(args.load):
        with open('data.pickle', 'rb') as f:
            # The protocol version used is detected automatically, so we do not
            # have to specify it.
            data = pickle.load(f)
    else:
       data = gen_counts(args, nclasses)

    from scipy.stats import spearmanr, pearsonr

    print (len(data["source_files"]), len(data["asi_1_pred"]))

    # Convert any tensors we have into values
    for i in range(len(data["asi_1_pred"])):
        data["asi_1_pred"][i] = data["asi_1_pred"][i].item()
        data["asi_2_pred"][i] = data["asi_2_pred"][i].item()
        data["asj_1_pred"][i] = data["asj_1_pred"][i].item()
        data["asj_2_pred"][i] = float(data["asj_2_pred"][i])

    print("Correlations - spearmans & pearsons - ASI-1, ASI-2, ASJ-1, ASJ-2")

    asi_1_cor = spearmanr(data["asi_1_actual"], data["asi_1_pred"])
    asi_2_cor = spearmanr(data["asi_2_actual"], data["asi_2_pred"])
    asj_1_cor = spearmanr(data["asj_1_actual"], data["asj_1_pred"])
    asj_2_cor = spearmanr(data["asj_2_actual"], data["asj_2_pred"])
    print(asi_1_cor, asi_2_cor, asj_1_cor, asj_2_cor)

    asi_1_cor = pearsonr(data["asi_1_actual"], data["asi_1_pred"])
    asi_2_cor = pearsonr(data["asi_2_actual"], data["asi_2_pred"])
    asj_1_cor = pearsonr(data["asj_1_actual"], data["asj_1_pred"])
    asj_2_cor = pearsonr(data["asj_2_actual"], data["asj_2_pred"])
    print(asi_1_cor, asi_2_cor, asj_1_cor, asj_2_cor)


    asi_combo_real =  [data['asi_1_actual'][i] + data['asi_2_actual'][i] for i in range(len(data['asi_1_actual']))]
    asi_combo_pred =  [data['asi_1_pred'][i] + data['asi_2_pred'][i] for i in range(len(data['asi_1_pred']))]
    asj_combo_real =  [data['asj_1_actual'][i] + data['asj_2_actual'][i] for i in range(len(data['asj_1_actual']))]
    asj_combo_pred =  [data['asj_1_pred'][i] + data['asj_2_pred'][i] for i in range(len(data['asj_1_pred']))]

    print("Correlations - spearmans & pearsons - ASI, ASJ")
    asi_combo_cor = spearmanr(asi_combo_real, asi_combo_pred)
    asj_combo_cor = spearmanr(asj_combo_real, asj_combo_pred)
    print(asi_combo_cor, asj_combo_cor)

    asi_combo_cor = pearsonr(asi_combo_real, asi_combo_pred)
    asj_combo_cor = pearsonr(asj_combo_real, asj_combo_pred)
    print(asi_combo_cor, asj_combo_cor)


    import matplotlib.pyplot as plt
    import seaborn as sns
    import pandas as pd

    sns.set_theme(style="darkgrid")
    fig, axes = plt.subplots(2, 2)

    individuals = list(range(len(data["asi_1_actual"])))
    df0 = pd.DataFrame({"individual": individuals, "asi_1_actual": data["asi_1_actual"], "asi_1_pred": data["asi_1_pred"]})
    df1 = pd.DataFrame({"individual": individuals, "asi_2_actual": data["asi_2_actual"], "asi_2_pred": data["asi_2_pred"]})
    df2 = pd.DataFrame({"individual": individuals, "asj_1_actual": data["asj_1_actual"], "asj_1_pred": data["asj_1_pred"]})
    df3 = pd.DataFrame({"individual": individuals, "asj_2_actual": data["asj_2_actual"], "asj_2_pred": data["asj_2_pred"]})

    sns.lineplot(x="individual", y='value', hue='variable', 
             data=pd.melt(df0, ['individual']), ax=axes[0][0])
  
    sns.lineplot(x="individual", y='value', hue='variable', 
             data=pd.melt(df1, ['individual']), ax=axes[0][1])

    sns.lineplot(x="individual", y='value', hue='variable', 
             data=pd.melt(df2, ['individual']), ax=axes[1][0])
  
    sns.lineplot(x="individual", y='value', hue='variable', 
             data=pd.melt(df3, ['individual']), ax=axes[1][1])
    
    plt.show()