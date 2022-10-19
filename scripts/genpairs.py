
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
                        original = tokens[0].replace("Renaming ","")[:-1]
                        derived = tokens[1][1:].replace("\n","")
                        w.write(original + "," + derived + "\n")