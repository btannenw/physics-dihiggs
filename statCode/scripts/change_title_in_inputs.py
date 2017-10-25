import sys
from os import listdir, makedirs, remove
from os.path import isfile, isdir, join
import errno
import subprocess
import shutil
import logging
import re
import ROOT

# python scripts/change_title_in_inputs.py inputs/21022 "TwoLepton_21tag" "_[1-2]tag" "_21tag"

def getall(d, basepath="/"):
    "Generator function to recurse into a ROOT file/dir and yield (path, obj) pairs"
    for key in d.GetListOfKeys():
        kname = key.GetName()
        if key.IsFolder():
            # TODO: -> "yield from" in Py3
            for i in getall(d.Get(kname), basepath+kname+"/"):
                yield i
        else:
            yield basepath+kname, d.Get(kname)

def main(input_directory, output_directory, input_file_regex,
         title_regex_match, title_replace):
    """
    """
    logging_format = '%(levelname)s in %(funcName)s at %(asctime)s: %(message)s'
    time_format = '%Y-%j-%H-%M-%S-%Z'
    logging.basicConfig(format=logging_format,datefmt=time_format,level=logging.INFO)

    onlyfiles = [f for f in listdir(input_directory) if (isfile(join(input_directory, f)) and '.root' in f)]

    file_list = []
    for f in onlyfiles:
        if input_file_regex.search(f):
            file_list.append(join(input_directory, f))

    for full_file_name in file_list:
        full_tmp_name = full_file_name+".backup"
        shutil.copy2(full_file_name, full_tmp_name)

        mod_file = ROOT.TFile.Open(full_file_name, 'RECREATE')
        old_file = ROOT.TFile.Open(full_tmp_name)

        # NOTE title changes here
        created_systematics_dir = False
        for name, obj in getall(old_file):
            if obj.InheritsFrom('TH1'):
                if 'Sys' in name:
                    if not created_systematics_dir:
                        mod_file.mkdir('Systematics')
                        created_systematics_dir = True
                    mod_file.cd('Systematics')
                else:
                    mod_file.cd('/')
                _name = obj.GetName()
                _title = obj.GetTitle()
                obj.SetName(title_regex_match.sub(title_replace, _name))
                obj.SetTitle(title_regex_match.sub(title_replace, _title))
                obj.Write()

        old_file.Close()
        mod_file.Close()
        remove(full_tmp_name)

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='Combine histograms for fit\
     inputs')
    parser.add_argument('input_directory', type=str,
                        help='path to raw histograms')
    parser.add_argument('input_file_regex', type=str,
                        help='file name regex to match')
    parser.add_argument('title_regex_match', type=str,
                        help='title regex to match')
    parser.add_argument('title_replace', type=str,
                        help='title portion replacement')
    args = parser.parse_args(sys.argv[1:])
    main(args.input_directory, args.input_directory,
         re.compile(args.input_file_regex), re.compile(args.title_regex_match),
         args.title_replace)
