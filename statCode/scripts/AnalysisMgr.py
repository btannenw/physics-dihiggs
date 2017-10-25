from collections import OrderedDict as odict
import sys
import re

class WorkspaceConfig(odict):
    """
    Class to store configuration for Workspace
    To be subclassed for each analysis
    """
    def __init__(self, *args, **kwargs):
        super(WorkspaceConfig, self).__init__()
        self.update(Years="2015", UseSystematics=False, UseStatSystematics=True, StatThreshold=0.05,
                    UseFloatNorms=False,  OneBin=False, MassPoint="125",
                    UsePseudoData=False, ReplicaCount=0,
                    DefaultFitConfig=False, MergeSamples=True, Combination=False,
                    DecorrSys="None", DecorrTypes=0, ForceBinning=0, DoInjection=0, DecorrPOI="", Debug=False,
                    DeleteNormFiles=True, DoShapePlots=False, DoSystsPlots=False, LogScalePlots=True,
                    SmoothingControlPlots=False, OverlayNominalForSystShapes=True,
                    LumiForAll=False
                    )

        # Set defaults
        self.update(kwargs)

        # Add some keyword configuration
        if "StatOnly" in args:
            self.stat_only()
        if "MCStat" in args:
            self.mc_stat()
        if "FloatOnly" in args:
            self.float_only()
        if "Systs" in args:
            self.systs()
        if "NormSys" in args:
            self.normSys()

        self.fmt = "{0:<30} {1}"
        self.channels = []

        self.regionRegexp = re.compile("^(\\d+)TeV_(\\w+)_(\\w+)tag(\\w+)jet_(\\d+)(_\\d+)?ptv(_\\w+)?_(\\w+)$")
        self.tagRegexp = re.compile("(\\d)([lmtx]{2})?(p)?")
        self.jetRegexp = re.compile("((\\d)(p)?fat)?(\\d)(p)?")
        self.descrRegexp = re.compile("(_([a-zA-Z0-9]+))+")

        return

    def stat_only(self):
        self["UseSystematic"] = False
        self["UseFloatNorms"] = False
        self["UseStatSystematics"] = False

    def mc_stat(self):
        self["UseStatSystematics"] = True

    def systs(self):
        self["UseSystematics"] = True
        self["UseFloatNorms"] = True

    def normSys(self):
        self["UseSystematics"] = False
        self["UseFloatNorms"] = True

    def float_only(self):
        self["UseSystematics"] = False
        self["UseFloatNorms"] = True

    def _sanitise(self, v):
        "Sanitise python types for writing to config file"
        if type(v) == type(True):
            val = 'true' if v else 'false'
        elif type(v) == type(list) or type(v) == type(tuple) or type(v) == type(set):
            val = ','.join([str(x) for x in set(v)])
        else:
            val = v

        return val

    def _line(self, key):
        "Write a single line with check if exists"
        try:
            return self.fmt.format(key, self[key])
        except KeyError:
            sys.exit("No {0} set ! Aborting...".format(key))

    def create_config(self, name, order=None):
        """
        Write the object to a config file
        InputVersion, Analysis and Years must exits and are written in the top
        Regions are added at the bottom
        All others go inbetween in th order inserted in the dictionary unless
        a list or space-separated string is provided with a given ordering
        """

        if order:
            order = order.split() if isinstance(order, basestring) else order
        else:
            order = self.keys()

        lines = []
        explicit = ["InputVersion", "Analysis", "Years"]
        for k in explicit:
            lines.append(self._line(k))

        lines.append("")

        for k in order:
            if k in explicit or k == 'Regions': continue
            val = self._sanitise(self[k])
            lines.append(self.fmt.format(k, val))

        lines.append("")

        try:
            regs = [self["Regions"]] if isinstance(self["Regions"], basestring) else self["Regions"]
        except KeyError:
            sys.exit("No regions set ! Aborting...")

        for r in regs:
            lines.append(self.fmt.format("+Regions", r))

        filename = "configs/" + name
        with open(filename, 'w') as f:
            f.write('\n'.join(lines))
            f.write('\n')

        return filename

    def write_configs(self, basename):
        """
        Write the configs to file
        """
        conf_names = []
        if self["UsePseudoData"] == False:
            self["ReplicaCount"] = 0
            conf_names.append(self.create_config(basename+".config", self))
        else:
            for i in range(self["ReplicaCount"]):
                self["ReplicaCount"] = i
                conf_names.append(self.create_config(basename+"_"+str(i)+".config", self))
        return conf_names

    def set_channels(self, chans):
        """
        Set a list of channel, which can be useful for implmenting regions
        based on which cahnnels you specific but is not used otherwise
        """
        chans = [chans] if isinstance(chans, basestring) else chans
        self.channels.extend(chans)

    def check_regions(self):
        """
        Check the regions match the regexp on the C++-side

        TODO add a check for duplicated regions and remove them
        """
        for r in self["Regions"]:
            m = self.regionRegexp.match(r)
            if m is None:
                sys.exit("Malformed region " + r)

            m2 = self.tagRegexp.match(m.groups()[2])
            if m2 is None:
                sys.exit("Malformed tag region " + m2.groups()[2])

            m3 = self.jetRegexp.match(m.groups()[3])
            if m is None:
                sys.exit("Malformed jet region " + m.groups()[3])

    def set_regions(self, regions = []):
        """
        Set the regions.  By default they are passed in as a list but
        the suggested behaviour is to overwrite this function in the
        analysis-specific derived class to add the correct regions based
        on channel, ...
        """
        self["+Regions"] = [regions] if isinstance(regions, basestring) else regions
        self.check_regions()

