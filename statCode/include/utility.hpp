#ifndef utility_hpp
#define utility_hpp

#include <vector>

#include <TString.h>

class TObjArray;
class TH1;

/**
 * @brief A few small general-purpose functions
 *
 * That don't depend on anything specific
 */
namespace Utils {

  /**
   * @brief Split a @c TString at occurrences of a separator
   *
   * @todo Could we use @c TString::Tokenize() to simplify it further ?
   *
   * @param orig The string to split
   * @param separator The separator character
   * @return A vector containing the substrings
   */
  std::vector<TString> splitString(TString orig, const char separator);

  /**
   * @brief Convert @c float to @c TString
   *
   * @todo Do we still need this function ?
   *
   * @param d	The @c float to convert
   * @return A @c TString representation of the @ float
   */
  TString ftos(float d);

  /**
   * @brief Get a specific @c TString item in a @c TObjArray
   *
   * Used to easily parse the results of @c TPRegexp matches
   *
   * @param res	The @c TObjArray of @c TString
   * @param i The index we want to look at
   * @return The @c TString at position @c i
   */
  TString group(TObjArray* res, int i); // get some group out of matching res

  /**
   * @brief Symmetrize an histogram wrt another
   *
   *     down = 2*nominal - up
   *
   * Used to symmetrize systematics for instance
   *
   * @param up	The histogram to symmetrize
   * @param nominal	The reference histogram
   * @return The symmetric of @c up wrt @c nominal
   */
  TH1* symmetrize(const TH1* up, const TH1* nominal);

}

#endif
