#/usr/bin/python

import sys

if len(sys.argv) is not 2:
    print "Usage: tableMaker.py <filename>"
    exit(0)

filename = sys.argv[1]

inFile = open (filename,'r')
lines = inFile.readlines()
inFile.close()
outname = filename.split('.txt')[0]
outname = outname + '.tex'
outFile = open (outname,'w')

outFile.write('\\begin{table}[h!]')
outFile.write('\centering')
#outFile.write('\fontsize{10}{11}\selectfont')
outFile.write('\\begin{tabular}{l|c}')
outFile.write('\hline\hline\n')

inlines = []
newline =[]
for line in lines:
    newline =[]
    
    # loop over lines and add text/numbers to newline when non-empty
    for part in line.split('\n')[0].split(' '):
        if part != '':
            newline.append(part)

    # add line to list of lines
    inlines.append(newline)

for line in inlines:
    if len(line) == 5:
        #print line[0] + "   &   " + line[2] + "   \pm   " + line[3] + "  \\\\\hline"
        outFile.write(line[0] + "   &   " + line[2] + "   \pm   " + line[3] + "  \\\\\hline\n")
    else:
        if 'bkg' in line[0]:
            #print 'hline'
            outFile.write('\hline ')
        #print line[0] + "   &   " + line[1]
        outFile.write(line[0] + "   &   " + line[1] + "  \\\\\hline\n")

outFile.write('\hline\hline\n\end{tabular}')
outFile.write('\end{table}')

outFile.close()

#print inlines
