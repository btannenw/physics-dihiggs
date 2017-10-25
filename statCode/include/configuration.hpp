#ifndef configuration_hpp
#define configuration_hpp

#include <memory>
#include <vector>
#include <unordered_map>

#include <TEnv.h>
#include <TString.h>

/**
 * @brief List of all known analyses using the framework
 *
 * The names defined here are used throughout the codebase to mark portions of code and
 * hacks that should be specific to one or several analyses
 *
 * This is also used by @c AnalysisHandler to build the correct @c Analysis
 *
 */

class AnalysesTypes {
  public:
    enum class AnalysisType : std::int8_t {
      None,
      VHbb,
      // VZbb, // maybe we should define the diboson as its own ?
      AZh,
      VHbbRun2,
      HVT,
      MonoH,
      Htautau,
      HH,
      HHWWbb,
      VBFGamma
    };

    static const std::unordered_map<TString, AnalysisType> names;

};

using AnalysisType = AnalysesTypes::AnalysisType;

/**
 * @brief This is basically a proxy to @c TEnv, with additional methods
 *
 * This class is a thin layer on top of a @c TEnv. The @c TEnv is embedded as a shared ptr,
 * so users (classes) can instantiate a @c Configuration instance rather than passing
 * a pointer around.
 *
 * Additionally, it contains two general purpose items, debug flag and version of the run
 */

class Configuration {
  protected:
    /// The shared ptr to the underlying TEnv
    std::shared_ptr<TEnv> m_env;

    /// Version/tag for the call to MakeWorkspace
    TString m_version;

    /// Debug flag
    static bool m_debug;

    /// Analysis type
    static AnalysisType m_analysis;

  public:
    /// No empty constructor
    Configuration()  = delete;

    /**
     * @brief Constructor for a new Configuration
     *
     * Constructor that will read the config file and instantiate the @c m_env pointer.
     *
     * Configuration switches that are read here are:
     * @conf{Debug, false} turn on debug mode
     * @conf{Analysis, None} chose the AnalysisType to run
     *
     * @param fname	@c /path/to/configfile.conf
     * @param version	A version number or tag.
     *
     */
    Configuration(const TString& fname, const TString& version);

    /**
     * @brief Copy constructor
     *
     * Create a new @c Configuration object that will share the @c m_env pointer.
     */
    Configuration(const Configuration&) = default;

    /**
     * @brief Move constructor
     *
     * @see Configuration(const Configuration&)
     *
     */
    Configuration(Configuration&&) = default;

    /// Destructor
    ~Configuration() {}

    /// @todo add functions without deflt value that throw when the config is not present
    /// i.e mandatory keywords.

    /// @name Accessors
    /// @{

    /**
     * @brief Get an @c int
     *
     * @param name	Parameter name in the @c TEnv
     * @param dflt	Default value if @c name is not found in the @c TEnv
     * @return The value found in the @c TEnv
     */
    inline int getValue(const char* name, const int dflt) const {
      return const_cast<TEnv*>(m_env.get())->GetValue(name, dflt);
    }

    /**
     * @brief Get a @c bool
     *
     * @param name	Parameter name in the @c TEnv
     * @param dflt	Default value if @c name is not found in the @c TEnv
     * @return The value found in the @c TEnv
     */
    inline bool getValue(const char* name, const bool dflt) const {
      return static_cast<bool>(const_cast<TEnv*>(m_env.get())->GetValue(name, static_cast<int>(dflt)));
    }

    /**
     * @brief Get a @c double
     *
     * @param name	Parameter name in the @c TEnv
     * @param dflt	Default value if @c name is not found in the @c TEnv
     * @return The value found in the @c TEnv
     */
    inline double getValue(const char* name, const double dflt) const {
      return const_cast<TEnv*>(m_env.get())->GetValue(name, dflt);
    }

    /**
     * @brief Get a @c TString
     *
     * @param name	Parameter name in the @c TEnv
     * @param dflt	Default value if @c name is not found in the @c TEnv
     * @return The value found in the @c TEnv
     */
    inline TString getValue(const char* name, const char* dflt) const {
      return const_cast<TEnv*>(m_env.get())->GetValue(name, dflt);
    }

    /**
     * @brief Get a comma-separated list of @c ints
     *
     * @param name	Parameter name in the @c TEnv
     * @return A vector of values, or an empty vector if @c name is not found
     */
    std::vector<int> getIntList(const char* name);

    /**
     * @brief Get a comma-separated list of @c ints
     *
     * @param name	Parameter name in the @c TEnv
     * @param dflt	A default value if @c name is not found in the @c TEnv
     * @return A vector of values, or a vector with 1 element, @c dflt, if @c name is not in
     * the @c TEnv
     */
    std::vector<int> getIntList(const char* name, const int dflt);

    /**
     * @brief Get a comma-separated list of @c doubles
     *
     * @param name	Parameter name in the @c TEnv
     * @return A vector of values, or an empty vector if @c name is not found
     */
    std::vector<double> getDoubleList(const char* name);

    /**
     * @brief Get a comma-separated list of @c doubles
     *
     * @param name	Parameter name in the @c TEnv
     * @param dflt	A default value if @c name is not found in the @c TEnv
     * @return A vector of values, or a vector with 1 element, @c dflt, if @c name is not in
     * the @c TEnv
     */
    std::vector<double> getDoubleList(const char* name, const double dflt);

    /**
     * @brief Get a comma-separated list of @c TString
     *
     * @param name	Parameter name in the @c TEnv
     * @return A vector of values, or an empty vector if @c name is not found
     */
    std::vector<TString> getStringList(const char* name);

    /**
     * @brief Get the list of analysis regions
     *
     * @return A vector of names, or an empty vector if the keyword @c Regions is not found
     */
    std::vector<TString> getRegions();
    /// @}

    /// Are we in debug mode ?
    static bool debug() { return m_debug; }

    /// Analysis type we are running
    static AnalysisType analysisType() { return m_analysis; }

    /// Version number/tag for this run
    const TString& version() const { return m_version; }

};

#endif
