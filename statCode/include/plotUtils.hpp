#ifndef plotUtils_hpp
#define plotUtils_hpp

#include <map>
#include <utility>
#include <set>
#include <vector>

#include <TString.h>

class TAxis;
class TGraph;
class TH1;

/**
 * @file plotUtils.hpp
 * @brief Holds PU namespace
 *
 * author: Nicolas Morange
 * date : 21/03/2013
 */

/**
 * @brief Namespace to hold utility functions to work with histograms / @c TGraphs with labelled bins
 *
 * Plots like pulls of NP, or showing the 'status' of systematics wrt pruning, are tyically
 * THx or TGraphs where bins have labels attached (names of NP, names of analysis regions...)
 *
 * This namespace provides functions to perform operations on such type of data:
 * * sorting of the axes in user-defined order
 * * extract sub-histos or sub-tgraphs that contain or exclude bins defined by patterns
 * * center all points of a @c TGraph around 0
 * * set all labels at once
 *
 * This namespace is defined in the @c Linkdef.h file, hence all functions are accessible from
 * pyROOT scripts.
 *
 * @todo Should this remain a namespace, or be a class (with only static functions) to properly
 * define which functions should be called by the users (public) and which are for internal use
 * only (private) ?
 */
namespace PU {
  /// @name Configurables
  /// @{

  /**
   * @brief Sorting order for samples used in the analysis
   *
   * @todo not up-to-date for VHbb
   */
  extern std::map<TString, int> samplesPriority;

  /**
   * @brief Sorting order for groups of systematics
   *
   * Is it actually used and up-to-date ?
   */
  extern std::map<TString, int> sysGroups;

  /// Pair of (label, position in axis)
  typedef std::pair<TString, int> lab_pos;

  /// Vector of pair of (label, position in axis)
  typedef std::vector<lab_pos> v_lab_pos;

  /// @}

  /// @name Main functions
  /// @{

  /**
   * @brief Sort in-place the @c x or @c y axis of a @c TH1 or @c TH2
   *
   * Sort axis of TH1 or TH2 according to ordering given by fcn, in-place
   *
   * @param h Histogram of which the axis should be sorted
   * @param isXaxis @c true to sort x-axis, @c false to sort y-axis
   * @param fcn Comparison function used to sort the axis
   * @param reverse Reverse sorting order
   */
  void sortTHAxis(TH1* h, const bool isXaxis, bool (*fcn)(const lab_pos&, const lab_pos&), bool reverse=false);

  /**
   * @brief Extract a sub-histo with bins matching some patterns EXCLUDED
   *
   * The name of the resulting histo is the name of the input histo with "_reduced" appended
   *
   * Patterns are matched simply if they are substrings of the bin labels
   *
   * @param h Histogram from which a sub-histo should be extracted
   * @param isXaxis @c true to work on x-axis, @c false to work on y-axis
   * @param excludes Vector of patterns that should be excluded from the result
   * @return A new histogram with bins matching the patterns excluded
   */
  TH1* reduceTHAxisExclude(const TH1* h, const bool isXaxis, const std::vector<TString>& excludes);

  /**
   * @brief Extract a sub-histo with ONLY bins matching some patterns
   *
   * The name of the resulting histo is the name of the input histo with "_reduced" appended
   *
   * Patterns are matched simply if they are substrings of the bin labels
   *
   * @param h Histogram from which a sub-histo should be extracted
   * @param isXaxis @c true to work on x-axis, @c false to work on y-axis
   * @param includes Vector of patterns that enter the result
   * @return A new histogram with bins matching the patterns included
   */
  TH1* reduceTHAxisInclude(const TH1* h, const bool isXaxis, const std::vector<TString>& includes);

  /**
   * @brief Sort in-place an axis and rearrange a @c TGraph accordingly
   *
   * The axis is supposed to match either the @c x or the @c y axis of the @c TGraph,
   * i.e having labelled bins that match the points of the @c TGraph
   *
   * The axis can belong to another object than the @c TGraph, like another @c TGraph.
   * This is useful because when several @c TGraph are plotted together it can happen that
   * only one of them has labels defined.
   * This is e.g the case for plots of pulls created in @c FitCrossCheckForLimits, where the
   * actual @c TGraph of the pulls does not have the labels attached
   *
   * @param g A @c TGraph to sort accordingly to an axis
   * @param a Axis with bin labels that should be sorted
   * @param isXaxis @c true to re-arrange the x-axis of the @c TGraph, @c false to re-arrange its y-axis
   * @param fcn Comparison function used to sort the axis
   * @param reverse Reverse sorting order
   */
  void sortTGraphAxis(TGraph* g, TAxis* a, const bool isXaxis, bool (*fcn)(const lab_pos&, const lab_pos&), const bool reverse=false);

  /**
   * @brief Extract a sub-graph with points matching some patterns EXCLUDED
   *
   * The axis is supposed to match either the @c x or the @c y axis of the @c TGraph,
   * i.e having labelled bins that match the points of the @c TGraph
   *
   * The axis can belong to another object than the @c TGraph, like another @c TGraph.
   * This is useful because when several @c TGraph are plotted together it can happen that
   * only one of them has labels defined.
   * This is e.g the case for plots of pulls created in @c FitCrossCheckForLimits, where the
   * actual @c TGraph of the pulls does not have the labels attached
   *
   * The name of the resulting @c TGraph is the name of the input one with "_reduced" appended
   *
   * Patterns are matched simply if they are substrings of the bin labels
   *
   * @param g A @c TGraph from which a sub-histo should be extracted
   * @param a Axis with bin labels that somehow match the @c TGraph
   * @param isXaxis @c true to work on the x-axis, @c false to work on the y-axis
   * @param excludes Vector of patterns that should be excluded from the result
   * @return A new @c TGraph with points matching the patterns excluded
   */
  TGraph* reduceTGraphAxisExclude(const TGraph* g, const TAxis* a, const bool isXaxis, const std::vector<TString>& excludes);

  /**
   * @brief Extract a sub-graph with ONLY bins matching some patterns
   *
   * The axis is supposed to match either the @c x or the @c y axis of the @c TGraph,
   * i.e having labelled bins that match the points of the @c TGraph
   *
   * The axis can belong to another object than the @c TGraph, like another @c TGraph.
   * This is useful because when several @c TGraph are plotted together it can happen that
   * only one of them has labels defined.
   * This is e.g the case for plots of pulls created in @c FitCrossCheckForLimits, where the
   * actual @c TGraph of the pulls does not have the labels attached
   *
   * The name of the resulting @c TGraph is the name of the input one with "_reduced" appended
   *
   * Patterns are matched simply if they are substrings of the bin labels
   *
   * @param g A @c TGraph from which a sub-histo should be extracted
   * @param a Axis with bin labels that somehow match the @c TGraph
   * @param isXaxis @c true to work on the x-axis, @c false to work on the y-axis
   * @param includes Vector of patterns that enter the result
   * @return A new @c TGraph with points matching the patterns included
   */
  TGraph* reduceTGraphAxisInclude(const TGraph* g, const TAxis* a, const bool isXaxis, const std::vector<TString>& includes);

  /**
   * @brief Shift slightly points of a TGraph along an axis
   *
   * All points of a @c TGraph are shifted along @c x or @c y by a user-defined value.
   *
   * This is useful to show side-by-side pulls from several fit results, to compare them.
   *
   * @param g @c TGraph whose points will be shifted
   * @param shift Size of the shift to apply
   * @param isXaxis @c true to shift along the x-axis, @c false to shift along the y-axis
   */
  void shiftTGraph(TGraph* g, const float shift, const bool isXaxis);

  /**
   * @brief Define a common axis for several @c TGraphs
   *
   * In order to show side-by-side fit results with different configurations,
   * it is necessary to find a common axis containing the labels from all of
   * them, then to place correctly all the fit results on this common axis.
   *
   * This function allows this, by performing the following steps:
   * * Record a set of labels from all input @c TGraphs
   * * Create an axis from the set
   * * Change all the input @c TGraphs so they match the new axis, by shifting
   * their points properly
   * * Sort the axis and all the @c TGraphs according to @c PU::comp_sysNames
   *
   * @param v Vector of @c TGraphs that will share a common axis
   * @param isXaxis @c true to work with x-axis, @c false to work with y-axis
   * @return An axis common to all @c TGraphs
   */
  TAxis* mergeTGraphAxis(const std::vector<TGraph*>& v, const bool isXaxis);

  /**
   * @brief Set the axis of a TGraph according to another axis
   *
   * Copy the input axis to set all labels of the @c TGraph at once
   *
   * @param g @c TGraph to modify
   * @param a Reference axis, whose labels will be copied to the graph axis
   * @param isXaxis @c true to work with x-axis, @c false to work with y-axis
   */
  void setTGraphAxis(TGraph* g, const TAxis* a, const bool isXaxis);

  /**
   * @brief Shift values of a @c TGraph to 0 for regular NP and 1 for norm factors
   *
   * The output @c TGraph has the same name as the input graph, with "_shifted"
   * appended.
   *
   * Points whose label starts with "norm_" are supposed to be floating norm NP
   * and are shifted to 1. All other points are shifted to 0.
   *
   * @param g Input @c TGraph to modify
   * @param a Axis with bin labels that somehow match the @c TGraph
   * @param isXaxis @c true to work with x-axis, @c false to work with y-axis
   * @return A copy of the input graph, with values shifted to 0 or 1
   */
  TGraph* shiftTGraphToZero(const TGraph* g, TAxis* a, const bool isXaxis);

  /// @}

  /// @name Utilities for TAxis
  /// @{

  /**
   * @brief Set bin labels of an axis
   *
   * Set labels of the bins of the axis from the set of names provided
   *
   * @param axis	Axis of which labels should be set
   * @param names Names of the labels
   */
  void SetBinLabels(TAxis* axis, const std::set<TString>& names);

  /**
   * @brief Set bin labels of an axis
   *
   * Set labels of the bins of the axis from the vector of names provided
   *
   * @param axis	Axis of which labels should be set
   * @param names Names of the labels
   */
  void SetBinLabels(TAxis* axis, const std::vector<TString>& names);

  /**
   * @brief Get all bin labels of an axis in a @c v_lab_pos form
   *
   * @c result is cleared before being filled.
   *
   * Get all labels of the bins of the axis in the form of a vector of
   * (label, position).
   *
   * @param axis	Axis of which labels are read
   * @param result Vector of pairs of (label, position)
   */
  void GetBinLabels(const TAxis* axis, v_lab_pos& result);

  /// @}

  /// @name Comparison functions
  /// @{

  /**
   * @brief Alphabetical comparison
   *
   * Simple alphabetical comparison of strings
   *
   * @param  p1	First label and its position
   * @param  p2 Second label and its position
   * @return @c true if @c label1<label2, @c false otherwise
   */
  bool comp_simple(const lab_pos& p1, const lab_pos& p2);

  /**
   * @brief Comparison according to @c sysGroups
   *
   * A label belongs to a sysGroup if it contains the sysGroup string.
   *
   * If sysGroup1!=sysGroup2, the order is that of the @c sysGroups priorities (lower
   * priority means it is <)
   *
   * If a label does not belong to any sysGroup, its priority is arbitrary high.
   *
   * If both labels are within the same sysGroup, the alphabetical ordering of the strings is used
   *
   * @param  p1	First label and its position
   * @param  p2 Second label and its position
   * @return @c true if @c label1<label2, @c false otherwise
   */
  bool comp_sysNames(const lab_pos& p1, const lab_pos& p2);

  /**
   * @brief Comparison of sample in category names
   *
   * @todo This function is obsolete, and has not been updated to follow VHbb Run1 Paper
   * naming conventions.
   * However, it is probably not in use any longer in the code
   *
   * @param  p1	First label and its position
   * @param  p2 Second label and its position
   * @return @c true if @c label1<label2, @c false otherwise
   */
  bool comp_fullSamplesNames(const lab_pos& p1, const lab_pos& p2);

  /// @}

  /// @name Utilites and internals
  /// @{

  /**
   * @brief Tokenize a "SampleInCategory" name, accordnig to EPS conventions
   *
   * @todo This function is obsolete, and only used by @c comp_fullSamplesNames
   *
   * @param  s	Name to tokenize
   * @param year Output year
   * @param channel Output channel
   * @param region Output region
   * @param bin Output bin
   * @param priority Output sample priority
   */
  void tokenize(const TString& s, TString& year, TString& channel, TString& region, TString&bin, int& priority);
  // reorder axis according to bins.second. bins.first is ignored
  /**
   * @brief Reorder the bins in one axis of a histogram
   *
   * Rearrange the contents of the histogram so that the order of the bins matches
   * that in @c bins.
   *
   * @c bins[i].second is the original position of what should be moved to bin @c i+1 (since ROOT
   * histograms start at bin 1)
   *
   * @param  h	Histogram whose bins should be reordered
   * @param isXaxis @c true to work on the x-axis, @c false to work on the y-axis
   * @param bins List of the bins of the axis, sorted in the new order
   */
  void shuffleTHAxis(TH1* h, const bool isXaxis, const v_lab_pos& bins);

  /**
   * @brief Reorder the bins in one axis of a histogram
   *
   * Rearrange the contents of the histogram so that the order of the bins matches
   * that in @c bins.
   *
   * @c bins[i] is the original position of what should be moved to bin @c i+1 (since ROOT
   * histograms start at bin 1)
   *
   * @param  h	Histogram whose bins should be reordered
   * @param isXaxis @c true to work on the x-axis, @c false to work on the y-axis
   * @param bins List of the bins of the axis, sorted in the new order
   */
  void shuffleTHAxis(TH1* h, const bool isXaxis, const std::vector<int>& bins);

  /**
   * @brief Extract a sub-histo including or excluding bins matching some patterns
   *
   * The name of the resulting histo is the name of the input histo with "_reduced" appended
   *
   * Patterns are matched simply if they are substrings of the bin labels
   *
   * @param h Histogram from which a sub-histo should be extracted
   * @param isXaxis @c true to work on x-axis, @c false to work on y-axis
   * @param patterns Vector of patterns that enter or are removed from the result
   * @param includes @c true to keep only bins that match @c patterns, @c false to keep all bins
   * except those matching @c patterns
   * @return A new histogram with bins matching the patterns included or excluded
   */
  TH1* reduceTHAxis(const TH1* h, const bool isXaxis, const std::vector<TString>& patterns, bool includes);

  /**
   * @brief Reorder in-place an axis and rearrange a @c TGraph accordingly
   *
   * The axis is supposed to match either the @c x or the @c y axis of the @c TGraph,
   * i.e having labelled bins that match the points of the @c TGraph
   *
   * The axis can belong to another object than the @c TGraph, like another @c TGraph.
   * This is useful because when several @c TGraph are plotted together it can happen that
   * only one of them has labels defined.
   * This is e.g the case for plots of pulls created in @c FitCrossCheckForLimits, where the
   * actual @c TGraph of the pulls does not have the labels attached
   *
   * @c bins[i].second is the original position of what should be moved to bin @c i+1 (since ROOT
   * axis start at bin 1)
   *
   * @param g A @c TGraph to rearrange accordingly to an axis
   * @param a Axis with bin labels that should be sorted
   * @param isXaxis @c true to re-arrange the x-axis of the @c TGraph, @c false to re-arrange its y-axis
   * @param bins List of the bins of the axis, sorted in the new order
   */
  void shuffleTGraphAxis(TGraph* g, TAxis* a, const bool isXaxis, const v_lab_pos& bins);

  /**
   * @brief Reorder in-place an axis and rearrange a @c TGraph accordingly
   *
   * The axis is supposed to match either the @c x or the @c y axis of the @c TGraph,
   * i.e having labelled bins that match the points of the @c TGraph
   *
   * The axis can belong to another object than the @c TGraph, like another @c TGraph.
   * This is useful because when several @c TGraph are plotted together it can happen that
   * only one of them has labels defined.
   * This is e.g the case for plots of pulls created in @c FitCrossCheckForLimits, where the
   * actual @c TGraph of the pulls does not have the labels attached
   *
   * @c bins[i] is the original position of what should be moved to bin @c i+1 (since ROOT
   * axis start at bin 1)
   *
   * @param g A @c TGraph to rearrange accordingly to an axis
   * @param a Axis with bin labels that should be sorted
   * @param isXaxis @c true to re-arrange the x-axis of the @c TGraph, @c false to re-arrange its y-axis
   * @param bins List of the bins of the axis, sorted in the new order
   */
  void shuffleTGraphAxis(TGraph* g, TAxis* a, const bool isXaxis, const std::vector<int>& bins);

  /**
   * @brief Extract a sub-graph including or excluding points matching some patterns
   *
   * The axis is supposed to match either the @c x or the @c y axis of the @c TGraph,
   * i.e having labelled bins that match the points of the @c TGraph
   *
   * The axis can belong to another object than the @c TGraph, like another @c TGraph.
   * This is useful because when several @c TGraph are plotted together it can happen that
   * only one of them has labels defined.
   * This is e.g the case for plots of pulls created in @c FitCrossCheckForLimits, where the
   * actual @c TGraph of the pulls does not have the labels attached
   *
   * The name of the resulting @c TGraph is the name of the input one with "_reduced" appended
   *
   * Patterns are matched simply if they are substrings of the bin labels
   *
   * @param g A @c TGraph from which a sub-histo should be extracted
   * @param a Axis with bin labels that somehow match the @c TGraph
   * @param isXaxis @c true to work on the x-axis, @c false to work on the y-axis
   * @param patterns Vector of patterns that enter or are removed from the result
   * @param includes @c true to keep only bins that match @c patterns, @c false to keep all bins
   * except those matching @c patterns
   * @return A new @c TGraph with points matching the patterns excluded
   */
  TGraph* reduceTGraphAxis(const TGraph* g, const TAxis* a, const bool isXaxis, const std::vector<TString>& patterns, const bool includes);

  /**
   * @brief Find the bins matching some patterns in an axis, then put bins to be kept at the beginning
   *
   * Go through the bin labels of the axis, and set them simple "0" or "1" tags whether they are to be
   * kept or not, if they match and depending on @c includes.
   *
   * Then sort the bins using these "0" and "1" tags: if @c includes is @c true, the bins matching the
   * patterns are put at the beginning. If @c includes is @c false, the bins matching the patterns are
   * put to the end.
   *
   * Finally, return the new ordering.
   *
   * @param  a	Axis with labelled bins
   * @param patterns Vector of patterns that should enter or be removed from the result
   * @param includes @c true to keep only bins that match @c patterns, @c false to keep all bins
   * except those matching @c patterns
   * @param nPresent Number of bins that match the patterns
   * @return Sorted list of the bins, with the bins to be kept placed at the beginning
   */
  v_lab_pos findAndSortBins(const TAxis* a, const std::vector<TString>& patterns, bool includes, int& nPresent);

  /**
   * @brief Remove points of a graph with coordinate above some threshold
   *
   * If @c isXaxis is @c true, remove all points in @c g with x-value above @c val.
   * If @c isXaxis is @c false, remove all points in @c g with y-value above @c val.
   *
   * @param  g	@c TGraph to filter
   * @param isXaxis @c true to work on x-axis, @c false to work on y-axis
   * @param val Threshold above which to remove points
   */
  void removeTGraphPointsAbove(TGraph* g, const bool isXaxis, double val);

  /// Static filling of @c sysGroups
  std::map<TString, int> setSysGroups();

  /// Static filling of @c samplesPriority
  std::map<TString, int> setPriorities();

  /// @}
}

#endif
