
""" # noqa
   ___           __________________  ___________
  / _/__  ____  / __/ ___/  _/ __/ |/ / ___/ __/
 / _/ _ \/ __/ _\ \/ /___/ // _//    / /__/ _/          # noqa
/_/ \___/_/   /___/\___/___/___/_/|_/\___/___/          # noqa
Author : Benjamin Blundell - k1803390@kcl.ac.uk

retrofit.py - retroactively generate the CSV files for a
particular dataset.

Only works with cropped images. Doesn't work with resized images (at present)

Example use:
python retrofit.py --dataset /media/proto_backup/wormz/queelim/dataset_2d_all_extreme

CSV Format for the main file:
original source, original mask, fits source, fits mask, annotation log, annotation dat,
    new source name, new mask name, ROI X, Y, Z, WidthHeight, Depth, background

CSV Format for the U-Net file:
    input, output

TODO - add resizing options eventually.

"""

import argparse
import os
import fnmatch
from tqdm import tqdm

# Bits to replace when looking for the fits files
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
    source_to_fitsmask = {}
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
                        source_to_fits[s] = s.replace(fits_rep[0], fits_rep[1]).replace("tiff", "fits")

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
            _ats = {}

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
                    _ats[mask] = original
                    _mts[dat] = original
                    source_to_log[original] = dat.replace("_2.dat", "_2.log")

            # ... and the fits version
            for s in source_to_mask.keys():
                m = source_to_mask[s]
    
                for fits_rep in fits_replacements:
                    if fits_rep[0] in m:
                        source_to_fitsmask[s] = m.replace(fits_rep[0], fits_rep[1]).replace("tiff", "fits")

            old_style = False
            # Now lets look for the ROIs.
            # Reported ROIs are bigger than actual often as we go big then small if we are augmenting.
            # Typically, ROI sizes are set on the command line.
            for lidx, line in enumerate(lines):
                # New style ROI
                if ",ROI," in line:
                    tokens = line.split(",")
                    filepath = tokens[0]
                    og = _ats[filepath]

                    if og in source_to_derived.keys():
                        roi = {}
                        roi["xs"] = int(tokens[2])
                        roi["ys"] = int(tokens[3])
                        roi["zs"] = int(tokens[4])
                        roi["xe"] = roi["xs"] + int(tokens[5])
                        roi["ye"] = roi["ys"] + int(tokens[5])
                        roi["ze"] = roi["zs"] + int(tokens[6])
                        source_to_roi[og] = roi
            
            # Old style ROI - bit iffy this one
                elif "ROI: " in line:
                    old_style = True
                    break

            if old_style:
                counted = []

                for lidx, line in enumerate(lines):
                    if "Masking: " in lines[lidx]:
                        dat = lines[lidx].replace("Masking: ","").replace("\n","")

                        for ridx in range(lidx+1, len(lines)):
                            if "ROI: " in lines[ridx] and ridx not in counted:
                                try:
                                    rline = lines[ridx]
                                    original = _mts[dat]
                                    tokens = rline.replace(" ", "").replace("ROI:","").split(",")
                                    roi = {}
                                    counted.append(ridx)
                                    roi["xs"] = int(tokens[0])
                                    roi["ys"] = int(tokens[1])
                                    roi["zs"] = int(tokens[2])
                                    roi["xe"] = roi["xs"] + int(tokens[3])
                                    roi["ye"] = roi["ys"] + int(tokens[3])
                                    roi["ze"] = roi["zs"] + int(tokens[4])
                                    source_to_roi[original] = roi
                                except Exception as e:
                                    print("Couldn't find ROI for line:", lines[lidx].replace("\n",""))

                                break

    else:
        print("No dataset.log found!")
    
    with open(args.dataset + "/master_dataset.csv", "w") as w:
        w.write("ogsource,ogmask,fitssource,fitsmask,annolog,annodat,newsource,newmask,roix,roiy,roiz,roiwh,roid,back\n")

        for k in source_to_derived.keys():
            # We have all the operations, but we need to consider augmentation.
            # We should read the directory for the items we might need
            print(k)

            actual_files = []
            if os.path.exists(source_to_derived[k]):
                actual_files.append((k, source_to_derived[k], source_to_newmask[k]))
            else:
                # Check for augmentation
                fpath = os.path.dirname(source_to_derived[k])
                fname = os.path.basename(source_to_derived[k]); fname = fname.split("_")[0]
                found_masks = find_files(fname + "*mask.fits", fpath)
                found_masks.sort()
                found_derived = find_files(fname + "*layered.fits", fpath)
                found_derived.sort()
                assert(len(found_masks) == len(found_derived))

                for fidx in range(len(found_masks)):
                    actual_files.append((found_derived[fidx], found_masks[fidx]))

            for actual in actual_files:
                try:
                    csv_line = k + "," + source_to_mask[k] + "," + source_to_fits[k] + "," + source_to_fitsmask[k] + \
                        "," + source_to_log[k] + "," + source_to_dat[k] + "," + actual[0] + "," + actual[1]

                    if k in source_to_roi.keys():
                        roi = source_to_roi[k]
                        roi_x_off = ((roi['xe'] - roi['xs']) - args.roiwh) / 2
                        roi_y_off = ((roi['xe'] - roi['xs']) - args.roiwh) / 2
                        roi_z_off = ((roi['ze'] - roi['zs']) - args.roid) / 2
                        csv_line += "," + str(roi['xs'] + roi_x_off) + "," + str(roi['ys'] + roi_y_off) + "," + str(roi['zs'] + roi_z_off) + "," + str(args.roiwh) + "," + str(args.roid)
                    else:
                        print("ROI Fail")
                        assert(False)
                        csv_line += ",0,0,0,0,0"

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