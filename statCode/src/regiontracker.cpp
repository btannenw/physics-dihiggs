#include "regiontracker.hpp"

bool RegionTracker::operator[](const std::string& prop) const {
  if(! m_bits.count(prop)) {
    return false;
  }
  return m_bits.at(prop);
}

bool& RegionTracker::operator[](const std::string& prop) {
  if(! m_bits.count(prop)) {
    m_bits[prop] = false;
  }
  return m_bits.at(prop);
}
