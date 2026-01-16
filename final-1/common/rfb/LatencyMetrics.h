/* Copyright (C) 2024 - Latency Metrics Export Module
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// -=- LatencyMetrics.h - Export and analyze latency metrics

#ifndef __RFB_LATENCYMETRICS_H__
#define __RFB_LATENCYMETRICS_H__

#include <rfb/LatencyTracker.h>
#include <string>

namespace rfb {

  class LatencyMetrics {
  public:
    // Export to JSON format for dashboard
    static std::string exportToJSON(const LatencyTracker& tracker);
    
    // Export to CSV format for logging
    static std::string exportToCSV(const LatencyTracker& tracker);
    
    // Export detailed statistics
    static std::string exportStatistics(const LatencyTracker& tracker);
    
    // Save to file
    static bool saveToFile(const LatencyTracker& tracker, 
                          const char* filename,
                          const char* format = "json"); // json, csv, stats
    
  private:
    // Helper functions
    static std::string doubleToString(double value, int precision = 2);
    static std::string getTimestampString(const struct timeval& tv);
  };

} // namespace rfb

#endif // __RFB_LATENCYMETRICS_H__
