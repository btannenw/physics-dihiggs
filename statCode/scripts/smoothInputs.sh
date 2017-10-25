#NOTE: command line arguments:
#      first (required): path to directory of split/merged inputs
#      second: vector of strings formatted in a special way that indicates what and how to smooth
#              string is a semicolon-separated list of the following (only first is required)
#                 comma-separated list of samples to merge and smooth
#                 comma-separated list of file name keyword(s) to logically "and" together in order to include to determining whether to smooth or not
#                 comma-separated list of file name keyword(s) to logically "or" together in order to exclude in determining whether to smooth or not
#                 bool that is true if the full histogram range is left untouched on left side (false to truncate at first bin with positive data) - default = true
#                 bool that is true if the full histogram range is left untouched on right side (false to truncate at last bin with positive data) - default = true
#                 double representing a KNN (in fraction of events) that scales the bandwidth (never decreases) - default = 0.0
#                 double representing an event factor that scales the bandwidth (increase for narrower bandwidths and decrease for larger bandwidths) - default = 1.0
#                 double representing the KNN parameter fractional variation (used in creating a systematic associated with smoothing) - default = 0.0
#                 double representing the event-factor parameter fractional variation (used in creating a systematic associated with smoothing) - default = 0.0
#                 string representing whether to use a logarithmic (log) or linear (lin) scale - default = lin
#                 double representing by which factor to reduce the number of grid points to use - default = 1.0
#      third (optional): Boolean value indicating whether or not to write new inputs

inputDir=inputs/21018_newSmoothed

#root -l -q -b "smoothinputs.C+(\"${inputDir}\", {\"Zcc,Zbc,Zbb;TwoLepton,SR;fat;false;true;0.005\"})"
#root -l -q -b "macros/smoothinputs.C+(\"${inputDir}\", {\"Zcc;TwoLepton,SR;fat;false;;0.0005\"}, true)"

root -l -q -b "macros/smoothinputs.C+(\"${inputDir}\", {\"Zcc;TwoLepton,SR,0tag;fat;false;;;20;0.8;0.8;log;4.0\"}, true)"
