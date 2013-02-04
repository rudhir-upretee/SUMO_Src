#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
@file    analyze_teleports.py
@author  Jakob Erdmann
@date    2012-11-20
@version $Id: analyze_teleports.py 13106 2012-12-02 13:44:57Z behrisch $

Extract statistics from the warning outputs of a simulation run for plotting.

SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
Copyright (C) 2008-2012 DLR (http://www.dlr.de/) and contributors
All rights reserved
"""
import os,sys
import re
from collections import defaultdict

def parse_log(logfile, edges=True, aggregate=3600):
    print "Parsing %s" % logfile
    reFrom = re.compile("lane='([^']*)'")
    reTime = re.compile("time=(\d*)\.")
    # counts per lane
    waitingCounts = defaultdict(lambda:0)
    collisionCounts = defaultdict(lambda:0)
    # counts per step
    waitingStepCounts = defaultdict(lambda:0)
    collisionStepCounts = defaultdict(lambda:0)
    for line in open(logfile):
        try:
            if "Warning: Teleporting vehicle" in line:
                edge = reFrom.search(line).group(1)
                time = reTime.search(line).group(1)
                if edges:
                    edge = edge[:-2]
                if "collision" in line:
                    collisionCounts[edge] += 1
                    collisionStepCounts[int(time) / aggregate] += 1
                else:
                    waitingCounts[edge] += 1
                    waitingStepCounts[int(time) / aggregate] += 1
        except:
            print sys.exc_info()
            sys.exit("error when parsing line '%s'" % line)

    return (waitingCounts, collisionCounts,
            waitingStepCounts, collisionStepCounts)


def print_counts(countDict, label):
    counts = [(v,k) for k,v in countDict.items()] 
    counts.sort()
    print counts
    print label, 'total:', sum(countDict.values())


def main(logfile):
    waitingCounts, collisionCounts, waitingStepCounts, collisionStepCounts = parse_log(logfile)
    print_counts(waitingCounts, 'waiting')
    print_counts(collisionCounts, 'collisions')
    # generate plot
    min_step = min(min(waitingStepCounts.keys()),
            min(collisionStepCounts.keys()))
    max_step = max(max(waitingStepCounts.keys()),
            max(collisionStepCounts.keys()))
    plotfile = logfile + '.plot'
    with open(plotfile, 'w') as f:
        f.write("# plot '%s' using 1:2 with lines title 'waiting', '%s' using 1:3 with lines title 'collisions'\n" % (
            plotfile, plotfile))
        for step in range(min_step, max_step + 1):
            print >>f, ' '.join(map(str,[step, waitingStepCounts[step], collisionStepCounts[step]]))

if __name__ == "__main__":
    main(*sys.argv[1:])
