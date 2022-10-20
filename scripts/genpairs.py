
""" # noqa
   ___           __________________  ___________
  / _/__  ____  / __/ ___/  _/ __/ |/ / ___/ __/
 / _/ _ \/ __/ _\ \/ /___/ // _//    / /__/ _/          # noqa
/_/ \___/_/   /___/\___/___/___/_/|_/\___/___/          # noqa
Author : Benjamin Blundell - k1803390@kcl.ac.uk

genpairs.py - retroactively generate the csv of original to new

Example use:
python genpairs.py --dataset /media/proto_backup/wormz/queelim/dataset_2d_basic

"""

import argparse
import os


if __name__ == "__main__":
    # Training settings
    parser = argparse.ArgumentParser(description="Image Pair CSV generation")
    parser.add_argument('--dataset', default="")

    args = parser.parse_args()

    if args.dataset != "" and os.path.exists(args.dataset + "/dataset.log"):
        with open(args.dataset + "/pairs.csv", "w") as w:
            w.write("original,derived\n")

            with open(args.dataset + "/dataset.log", 'r') as f:
                lines = f.readlines()

                for line in lines:
                    if "Renaming" in line:
                        tokens = line.split(" to ")
                        original = tokens[0].replace("Renaming ","")
                        derived = tokens[1][1:].replace("\n","")
                        w.write(original + "," + derived + "\n")
        
        with open(args.dataset + "/pairs_fits.csv", "w") as w:
            w.write("original,derived\n")

            with open(args.dataset + "/dataset.log", 'r') as f:
                lines = f.readlines()

                for line in lines:
                    if "Renaming" in line:
                        tokens = line.split(" to ")
                        original = tokens[0].replace("Renaming ","")

                        original = original.replace("ins-6-mCherry/", "mcherry_fits/")
                        original = original.replace("ins-6-mCherry_2/", "mcherry_2_fits/")
                        original = original.replace(".tiff", ".fits")

                        derived = tokens[1][1:].replace("\n","")
                        w.write(original + "," + derived + "\n")

        with open(args.dataset + "/rois.csv", "w") as w:
            w.write("filename,x,y,z,xydim,zdim\n")

            with open(args.dataset + "/dataset.log", 'r') as f:
                lines = f.readlines()
                current_file = ""

                for line in lines:
                    if "Stacking" in line:
                        current_file = line.replace("Stacking: ","").replace("\n","") 

                    if "ROI:" in line and current_file != "":
                        tokens = line.split(",")
                        x = int(tokens[0].replace("ROI: ", ""))
                        y = int(tokens[1].replace(" ", ""))
                        z = int(tokens[2].replace(" ", ""))
                        xydim = int(tokens[3].replace(" ", ""))
                        zdim = int(tokens[4].replace(" ", "").replace("\n",""))
                        w.write(current_file + "," + str(x) + "," + str(y) + "," + str(z) + "," + str(xydim) + "," + str(zdim) + "\n")
                    elif ",ROI," in line:
                        tokens = line.split(",")
                        current_file = tokens[0]
                        x = int(tokens[2])
                        y = int(tokens[3])
                        z = int(tokens[4])
                        xydim = int(tokens[5])
                        zdim = int(tokens[6])
                        w.write(current_file + "," + str(x) + "," + str(y) + "," + str(z) + "," + str(xydim) + "," + str(zdim) + "\n")
