
""" # noqa
   ___           __________________  ___________
  / _/__  ____  / __/ ___/  _/ __/ |/ / ___/ __/
 / _/ _ \/ __/ _\ \/ /___/ // _//    / /__/ _/          # noqa
/_/ \___/_/   /___/\___/___/___/_/|_/\___/___/          # noqa
Author : Benjamin Blundell - k1803390@kcl.ac.uk

retrofit.py - retroactively generate the CSV files for a
particular dataset.

Example use:
python retrofit.py

CSV Format for the main file:
original source, original mask, fits source, annotation log, annotation dat,
    new source name, new mask name, ROI X, Y, Z, WidthHeight, Depth, background

CSV Format for the U-Net file:
    input, output

"""

import argparse
import os
import fnmatch


fits_replacements = [("ins-6-mCherry/", "mcherry_fits/"), ("ins-6-mCherry_2/", "mcherry_2_fits/")]

def find_files(pattern, path):
    result = []

    for root, dirs, files in os.walk(path):
        for name in files:
            if fnmatch.fnmatch(name, pattern):
                result.append(os.path.join(root, name))

    return result


def retrofit(args):
    source_to_derived = {}
    source_to_fits = {}
    source_to_roi = {}
    source_to_back = {}
    source_to_mask = {}
    source_to_newmask = {}
    source_to_dat = {}
    source_to_log = {}
    
    # First, start with the basic repairing
    if args.dataset != "" and os.path.exists(args.dataset + "/dataset.log"):

        with open(args.dataset + "/dataset.log", 'r') as f:
            lines = f.readlines()

            for line in lines:
                if "Renaming" in line:
                    tokens = line.split(" to ")
                    original = tokens[0].replace("Renaming ","")
                    derived = tokens[1].replace("\n","")

                    source_to_derived[original] = derived
                    source_to_newmask[original] = derived.replace("_layered", "_mask")

            # Now setup the source fits files    
            for s in source_to_derived.keys():
                for fits_rep in fits_replacements:
                    if fits_rep[0] in s:
                        source_to_fits[s] = s.replace(fits_rep[0], fits_rep[1])

            # Now find the background
            for lidx, line in enumerate(lines[-1]):
                if "Renaming" in line:
                    tokens = line.split(" to ")
                    original = tokens[0].replace("Renaming ","")
                    
                    if "Background" in lines[lidx+1]:
                        tokens2 = lines[lidx+1].split(":")
                        source_to_back[original] = int(tokens2[1])
                    else:
                        source_to_back[original]  = 0

            _mts = {}

            # Now find the mask file used
            for line in lines:
                if "Pairing" in line:
                    tokens0 = line.split(" with ")
                    tokens1 = tokens0[1].split(" and ")
                    mask = tokens0[0].replace("Pairing ", "")
                    dat = tokens1[0].replace("\n","")
                    original = tokens1[1].replace("\n","")
                    source_to_mask[original] = mask
                    source_to_dat[original] = dat
                    source_to_log[original] = dat.replace("_2.dat", "_2.log")
                    _mts[dat] = original

            # Now lets look for the ROIs.
            # Reported ROIs are bigger than actual often as we go big then small if we are augmenting.
            # Typically, ROI sizes are set on the command line.
            for lidx, line in enumerate(lines):
                # New style ROI
                if ",ROI," in line:
                    tokens = line.split(",")
                    filepath = tokens[0]

                    if filepath in source_to_derived.keys():
                        roi = {}
                        roi["xs"] = int(tokens[2])
                        roi["ys"] = int(tokens[3])
                        roi["zs"] = int(tokens[4])
                        roi["xe"] = roi["xs"] + int(tokens[5])
                        roi["ye"] = roi["ys"] + int(tokens[5])
                        roi["ze"] = roi["zs"] + int(tokens[6])
                        source_to_roi[filepath] = roi
                # Old style ROI
                elif "ROI: " in line:
                    if "Masking: " in lines[lidx-1]:
                        dat = lines[lidx-1].replace("Masking: ","").replace("\n","")
                        try:
                            original = _mts[dat]
                            tokens = line.replace(" ", "").replace("ROI:","").split(",")
                            roi = {}
                            roi["xs"] = int(tokens[0])
                            roi["ys"] = int(tokens[1])
                            roi["zs"] = int(tokens[2])
                            roi["xe"] = roi["xs"] + int(tokens[3])
                            roi["ye"] = roi["ys"] + int(tokens[3])
                            roi["ze"] = roi["zs"] + int(tokens[4])
                            source_to_roi[original] = roi
                        except:
                            print("Couldn't find ROI for line:", lines[lidx-1].replace("\n",""))


    with open(args.dataset + "/master_dataset.csv", "w") as w:
        w.write("ogsource,ogmask,fitssource,annolog,annodat,newsource,newmask,roix,roiy,roiz,roiwh,roid,back\n")
        
        for k in source_to_derived.keys():
            # We have all the operations, but we need to consider augmentation.
            # We should read the directory for the items we might need

            actual_files = []
            if os.path.exists(source_to_derived[k]):
                actual_files.append((k, source_to_derived[k], source_to_newmask[k]))
            else:
                # Check for augmentation
                fpath =  os.path.dirname(source_to_derived[k])
                fname =  os.path.basename(source_to_derived[k]); fname = fname.split("_")[0]
                found_masks = find_files(fname + "*mask.fits", fpath)
                found_masks.sort()
                found_derived = find_files(fname + "*layered.fits", fpath)
                found_derived.sort()
                assert(len(found_masks) == len(found_derived))

                for fidx in range(len(found_masks)):
                    actual_files.append((found_derived[fidx], found_masks[fidx]))

            for actual in actual_files:
                try:
                    csv_line = k + "," + source_to_mask[k] + "," + source_to_fits[k] + "," + source_to_log[k] + \
                        "," + source_to_dat[k] + "," + actual[0] + "," + actual[1]

                    if k in source_to_roi.keys():
                        roi = source_to_roi[k]
                        roi_x_off = ((roi['xe'] - roi['xs']) - args.roiwh) / 2
                        roi_y_off = ((roi['xe'] - roi['xs']) - args.roiwh) / 2
                        csv_line += "," + str(roi['xs'] + roi_x_off) + "," + str(roi['ys'] + roi_y_off) + "," + str(args.roiwh) + "," + str(args.roid)
                    else:
                        csv_line += ",0,0,0,0"

                    if k in source_to_back.keys():
                        csv_line += "," + str(source_to_back[k]) + "\n"
                    else:
                        csv_line += ",0\n"

                    w.write(csv_line)
                except:
                    print("Not all keys found for", k)


def create_unet_csv(args):
    infiles = []
    outfiles =[]

    for x in os.listdir(args.dataset):
        if x.endswith(".fits") and "layered" in x:
            infiles.append(x)
        elif x.endswith(".fits") and "mask" in x:
            outfiles.append(x)

    infiles.sort()
    outfiles.sort()

    with open(args.dataset + "/unet_dataset.csv", "w") as f:
        for idx in range(len(infiles)):
            f.write(infiles[idx] + "," + outfiles[idx] + "\n")


if __name__ == "__main__":
    # Training settings
    parser = argparse.ArgumentParser(description="Create a set of CSVs for our datasets, retrofitting from the logs.")
    parser.add_argument('--dataset', default="", help="The path to the dataset we want to retrofit.")
    parser.add_argument('--roiwh',  default=200, type=int)
    parser.add_argument('--roid',  default=51, type=int)

    args = parser.parse_args()
    retrofit(args)
    create_unet_csv(args)