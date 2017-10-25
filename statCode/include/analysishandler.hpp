#ifndef analysishandler_hpp
#define analysishandler_hpp

#include <memory>

class Analysis;
class Configuration;


/**
 * @brief Class responsible to build and hold an instance of @c Analysis
 *
 * This class is a small factory to build and give access to to an
 * instance of @c Analysis, based on the current @c analysisType
 *
 * All headers of the derived specialized classes should be added in
 * the .cpp (e.g @c binning_vhres.hpp, etc...)o
 *
 * For readability, please copy the current scheme when adding an
 * analysis in the implementation of the constructor, i.e
 * * Add a @c using statement
 * * Use it in the @c switch
 */
class AnalysisHandler
{
  protected:
    /// Global pointer to the @c Analysis
    static std::unique_ptr<Analysis> m_analysis;

  public:
    /// No empty constructor
    AnalysisHandler() = delete;

    /**
     * @brief Standard constructor
     *
     * Standard constructor with a copy of the @c Configuration.
     *
     * Based on @c Configuration::analysisType, creates the @c m_analysis
     * instance.
     *
     * @param conf	The reference configuration
     */
    AnalysisHandler(const Configuration& conf);

    /// Default copy constructor
    AnalysisHandler(const AnalysisHandler& other) = default;

    /// Destructor
    ~AnalysisHandler();

    /// Static accessor the the @c Analysis instance
    static Analysis& analysis() { return *m_analysis; }

};


#endif
