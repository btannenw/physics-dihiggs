#ifndef properties_hpp
#define properties_hpp

#include <unordered_map>
#include <algorithm>
#include <cstddef>

#include <TString.h>

/**
 * @file properties.hpp
 * @brief Description of the @c Properties and @c PropertiesSet
 *
 * @c Property and @c PropertiesSet are the basic blocks to describe the
 * analyses categories. Using this logic allows to easily refer to several
 * regions at once using common properties, e.g all 1-lepton 2-jet regions.
 * This is used super intensively to assign systematics to specific regions,
 * to work out the decorrelations of systematics based on the categories they
 * are in.
 */

/**
 * @brief Class/namespace to list the properties that describe the analysis regions
 *
 * Regions of the analysis are described by set of properties.
 * This class simply holds all the existing properties and their names.
 * Properties are typically the number of jets, of leptons, of tagged jets,
 * the pTV bin, the data-taking year, etc...
 * It should be extended if needed
 *
 */
class Properties {
  public:
    /// The list of existing properties
    enum class Property : std::int8_t {
      year, ///< Analysis year
      nLep, ///< Number of leptons
      spec, ///< Special things. topemucr, topcr...
      nJet, ///< Number of jets
      nFatJet, ///< Number of fat jets
      nTag, ///< Number of b-tags
      nAddTag, ///< Number of b-tags in additional jets
      tagType, /**< Type of b-tag "" for inclusive, @ ll, @c mm, @c tt,
                 or @c xx @c == @c mm @c + @c tt */
      bin, /**< @c pTV bin. 0: 0-90 for CUT or 0-120 for MVA, 1:90-120 or 100-120
             for 0 lepton, 2: 120-160 for CUT or >120 for MVA,
             3: 160-200, 4: >200 */
      hiLoMass, // whether selection strategy is low mass (opt700) or high mass (opt2000)
      binMin, ///< min of @c pTV bin.
      binMax, ///< max of @c pTV bin.
      lepFlav, ///< 0: mu, 1: e
      lepSign, ///< 0: neg, 1: pos
      type, ///< 0: CUT, 1: MVA
      dist, ///< Distribution (mjj, mva, ...)
      highVhf, ///< low/high cut on Vhf BDT
      descr, ///< description of the region
      incTag, /// Is inclusive in number of tags
      incJet, /// Is inclusive in number of jets
      incFat, /// Is inclusive in number of fat jets
      incAddTag, /// Is inclusive in number of additional b-tagged track jets
      nPh /// Number of photons
    };

    /// Map holding the names (as @c TString) of the properties
    static const std::unordered_map<Property, TString> names;

    /// Reverse map to get properties from their names
    static const std::unordered_map<TString, Property> props_from_names;

};

using Property = Properties::Property;

// necessary to use std::unordered_*<Property>
namespace std {
  /// Define hash<Property> so we can use properties in unordered containers
  template <> struct hash<Property>
  {
    std::size_t operator()(Property const& s) const {
      return std::hash<int>()(static_cast<int>(s));
    }
  };
}

typedef std::unordered_map<Property, int> int_prop_map;
typedef std::unordered_map<Property, TString> string_prop_map;

/**
 * @brief A set of properties
 *
 * A couple of maps of properties associated with their values, either @c int or @c TString.
 * A @c PropertiesSet is typically associated with each analysis region (@c Category) to
 * describe it uniquely.
 * A @c PropertiesSet can hold a limited set of properties, which can be then tested for
 * matches with the categories. This is the basic mechanism used with @c SysConfig to
 * assign systematics only in certain regions, and decorrelate systematics in some regions.
 *
 */
struct PropertiesSet {
/// @todo Note: implementation of PropertiesSet using boost::variant<int, string> looks
/// possible it might simplify operations further along, but so far the move looks a bit risky

  /// Map of properties with integer values
  int_prop_map intProperties;

  /// Map of properties with string values
  string_prop_map stringProperties;

  /**
   * @brief Constructor from maps
   *
   * Basic constructor from existing maps *that are moved*
   * Use-case is "declarative" construction of @c PropertiesSet as in
   * @c SystematicHandler. Example:
   *
   *     { { {nTag, 2}, {nJet, 3} }, { {dist, "mva"}, {spec, "topcr"} } }
   *
   * @param intProps A map of @c Property associated with @c int values
   * @param stringProps A map of @c Property associated with @c TString values
   */
  PropertiesSet(int_prop_map&& intProps, string_prop_map&& stringProps):
    intProperties(std::move(intProps)),
    stringProperties(std::move(stringProps))
  {}

  /**
   * @brief Constructor for simple case of 1 integer property
   *
   * In many cases e.g in @c SystematicHandler we need a simple PropertiesSet with
   * only 1 @c Property. The constructor is to simplify this common case. Example:
   *
   *     {nJet, 2}
   *
   * @param p	The @c Property
   * @param val	Its value
   */
  PropertiesSet(Property p, int val) :
  intProperties(), stringProperties()
  {
    intProperties[p] = val;
  }

  /**
   * @brief Constructor for simple case of 1 string property
   *
   * In many cases e.g in @c SystematicHandler we need a simple PropertiesSet with
   * only 1 @c Property. The constructor is to simplify this common case. Example:
   *
   *     {spec, "topcr"}
   *
   * @param p	The @c Property
   * @param val	Its value
   */
  PropertiesSet(Property p, const TString& val) :
  intProperties(), stringProperties()
  {
    stringProperties[p] = val;
  }

  /// Default empty constructor
  PropertiesSet() = default;

  /// Default copy constructor
  PropertiesSet(const PropertiesSet&) = default;

  /// Default move constructor
  PropertiesSet(PropertiesSet&&) = default;

  /// Default destructor
  ~PropertiesSet() = default;

  /**
   * @brief Copy the PropertiesSet with the exception of 1 integer property
   *
   * Copy @c this PropertiesSet into a new one, with the exception of 1 @c Property,
   * whose value is changed.
   * Typical use is to find the 3jet region corresponding to a given 2jet one.
   *
   * @param p	The @c Property that changes
   * @param i The new value of the property
   * @return The copy of the @c PropertiesSet
   */
  PropertiesSet copyExcept(const Property p, int i) const;

  /**
   * @brief Copy the PropertiesSet with the exception of 1 string property
   *
   * Copy @c this PropertiesSet into a new one, with the exception of 1 @c Property,
   * whose value is changed.
   * Typical use is to find the 3jet region corresponding to a given 2jet one.
   *
   * @param p	The @c Property that changes
   * @param s The new value of the property
   * @return The copy of the @c PropertiesSet
   */
  PropertiesSet copyExcept(const Property p, const TString& s) const;

  /**
   * @brief Check for existence of a @c Property
   *
   * Check if a given @c Property has been defined in the current @c PropertiesSet
   *
   * @param p	The @c Property to check
   * @return @c true if the @c Property has been defined. @c false otherwise.
   */
  bool hasProperty(const Property p) const;

  /// @name Accessors that returns a default value if property is not present
  /// @{

  /**
   * @brief Access an integer @c Property
   *
   * Get the value of an integer @c Property if it has been defined.
   * Returns -1 if the @c Property is not in the PropertiesSet or has been set as a
   * string @c Property
   *
   * @param p The @c Property to access
   * @return The value of @c p if @c p is in the @c PropertiesSet and has been set has an
   * integer @c Property. -1 otherwise.
   */
  int getIntProp(const Property p) const;

  /**
   * @brief Access a string @c Property
   *
   * Get the value of a string @c Property if it has been defined.
   * Returns -1 if the @c Property is not in the PropertiesSet or has been set as a
   * integer @c Property
   *
   * @param p The @c Property to access
   * @return The value of @c p if @c p is in the @c PropertiesSet and has been set has a
   * string @c Property. "" otherwise.
   */
  TString getStringProp(const Property p) const;

  /// Alias to @c getIntProp(p)
  inline int operator[](const Property p) const { return getIntProp(p); }

  /// Alias to @c getStringProp(p)
  inline TString operator()(const Property p) const { return getStringProp(p); }

  /// @}

  /// @name Accessors that throw an exception if property is not present
  /// @{

  /**
   * @brief Access an integer @c Property
   *
   * Get the value of an integer @c Property if it has been defined.
   * Throw an exception if the @c Property is not in the PropertiesSet or has been set as a
   * string @c Property
   *
   * @param p The @c Property to access
   * @return The value of @c p if @c p is in the @c PropertiesSet and has been set has an
   * integer @c Property. Throws otherwise.
   */
  int requestIntProp(const Property p) const { return intProperties.at(p); }

  /**
   * @brief Access a string @c Property
   *
   * Get the value of a string @c Property if it has been defined.
   * Throw an exception if the @c Property is not in the PropertiesSet or has been set as a
   * int @c Property
   *
   * @param p The @c Property to access
   * @return The value of @c p if @c p is in the @c PropertiesSet and has been set has a
   * string @c Property. Throws otherwise.
   */
  TString requestStringProp(const Property p) const { return stringProperties.at(p); }

  /// @}

  /// @name Setters
  /// @{

  /**
   * @brief Set the value of an integer @c Property
   *
   * @param p	The @c Property to set
   * @param i	The value of @c p
   */
  void setIntProp(Property p, int i);

  /**
   * @brief Set the value of a string @c Property
   *
   * @param p	The @c Property to set
   * @param s	The value of @c p
   */
  void setStringProp(Property p, const TString& s);

  /**
   * @brief Access a reference to an integer @c Property
   *
   * WHY DOES THIS EVEN COMPILE ?
   * Looks like an overloading based on return type only, which is not C++...
   * Is it actually used in the code ?
   *
   * @param p	The @c Property to access
   * @return A reference to @p
   */
  inline int& operator[](const Property p) { return intProperties[p]; }

  /**
   * @brief Access a reference to a string @c Property
   *
   * WHY DOES THIS EVEN COMPILE ?
   * Looks like an overloading based on return type only, which is not C++...
   * Is it actually used in the code ?
   *
   * @param p	The @c Property to access
   * @return A reference to @p
   */
  inline TString& operator()(const Property p) { return stringProperties[p]; }

  /// @}

  /**
   * @brief Check if @c this matches a given @c PropertiesSet
   *
   * Matching of @c PropertiesSet to another is used heavily in @c SystematicHandler
   * both to restrict some systs to some regions, and to define the decorrelations.
   *
   * Matching means:
   * All properties defined in @c pset are present in @c this and have the same value.
   *
   * @param pset	The @c PropertiesSet to check the matching against
   * @return @c true if @c this matches @c pset. @c false otherwise.
   */
  bool match(const PropertiesSet& pset) const;

  /**
   * @brief Merge another @c PropertiesSet into @c this
   *
   * Merging means:
   * All properties defined in @c other are set in @c this with the same value.
   *
   * If a @c Property defined in @c other already exists in @c this, its value is
   * overwritten without any warning.
   *
   * @param other	The @c PropertiesSet to merge into @c this
   */
  void merge(const PropertiesSet& other);

  /// Prints the contents of @c this to @c stdout
  void print() const;

  /**
   * @brief Get the string tag for a @c Property
   *
   * The tags are of the form:
   *
   *     _NameValue
   *
   * where @c Name is the name of @c p as defined in @c Properties::names, and
   * @c Value is the value associated to @c p in @c this.
   *
   * @param p	The @c Property to look at
   * @return The tag associated to @c p. Throws an exception if @c p does not exist in @c this.
   */
  TString getPropertyTag(const Property p) const;

  /**
   * @brief Get the tag associated to @c this
   *
   * The tag is of the form:
   *
   *     _P1V1_P2V2_P3V3...
   *
   * where the @c Pi are the properties defined in @c this, and the @c Vi are their values.
   *
   * @return The tag associated to @c this
   */
  TString getPropertiesTag() const;
};


#endif
