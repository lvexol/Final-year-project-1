/* Copyright (C) 2024 - Latency Metrics Export Implementation
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

// -=- LatencyMetrics.cxx - Export latency metrics

#include <rfb/LatencyMetrics.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <cstdio>
#include <cstring>

using namespace rfb;

std::string LatencyMetrics::doubleToString(double value, int precision)
{
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(precision) << value;
  return stream.str();
}

std::string LatencyMetrics::getTimestampString(const struct timeval& tv)
{
  char buffer[32];
  time_t t = tv.tv_sec;
  struct tm* tm_info = localtime(&t);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
  return std::string(buffer);
}

std::string LatencyMetrics::exportToJSON(const LatencyTracker& tracker)
{
  std::ostringstream json;
  auto stats = tracker.getStatistics();
  
  char timestamp[32];
  time_t now = std::time(NULL);
  struct tm* tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
  
  json << "{\n";
  json << "  \"timestamp\": \"" << timestamp << "\",\n";
  json << "  \"click_latency\": {\n";
  json << "    \"last\": " << doubleToString(tracker.getLastClickLatency()) << ",\n";
  json << "    \"average\": " << doubleToString(stats.avgClickLatency) << ",\n";
  json << "    \"minimum\": " << doubleToString(stats.minClickLatency) << ",\n";
  json << "    \"maximum\": " << doubleToString(stats.maxClickLatency) << ",\n";
  json << "    \"count\": " << stats.clickCount << "\n";
  json << "  },\n";
  json << "  \"display_latency\": {\n";
  json << "    \"last\": " << doubleToString(tracker.getLastDisplayLatency()) << ",\n";
  json << "    \"average\": " << doubleToString(stats.avgDisplayLatency) << ",\n";
  json << "    \"minimum\": " << doubleToString(stats.minDisplayLatency) << ",\n";
  json << "    \"maximum\": " << doubleToString(stats.maxDisplayLatency) << ",\n";
  json << "    \"count\": " << stats.displayCount << "\n";
  json << "  },\n";
  json << "  \"detailed_records\": [\n";
  
  const auto& latencies = tracker.getAllLatencies();
  for (size_t i = 0; i < latencies.size(); ++i) {
    const auto& data = latencies[i];
    json << "    {\n";
    json << "      \"id\": " << data.id << ",\n";
    json << "      \"type\": \"" << (data.eventType == 0 ? "click" : "display") << "\",\n";
    json << "      \"latency_ms\": " << doubleToString(data.latencyMs) << ",\n";
    json << "      \"timestamp\": \"" << getTimestampString(data.timestamp) << "\"\n";
    json << "    }";
    if (i < latencies.size() - 1) json << ",";
    json << "\n";
  }
  
  json << "  ]\n";
  json << "}\n";
  
  return json.str();
}

std::string LatencyMetrics::exportToCSV(const LatencyTracker& tracker)
{
  std::ostringstream csv;
  
  // Header
  csv << "ID,Type,Latency(ms),Timestamp\n";
  
  // Data
  const auto& latencies = tracker.getAllLatencies();
  for (const auto& data : latencies) {
    csv << data.id << ","
        << (data.eventType == 0 ? "click" : "display") << ","
        << doubleToString(data.latencyMs) << ","
        << getTimestampString(data.timestamp) << "\n";
  }
  
  return csv.str();
}

std::string LatencyMetrics::exportStatistics(const LatencyTracker& tracker)
{
  std::ostringstream stats_str;
  auto stats = tracker.getStatistics();
  
  stats_str << "=== LATENCY STATISTICS ===\n\n";
  
  stats_str << "CLICK LATENCY:\n";
  stats_str << "  Count: " << stats.clickCount << "\n";
  stats_str << "  Average: " << doubleToString(stats.avgClickLatency) << " ms\n";
  stats_str << "  Minimum: " << doubleToString(stats.minClickLatency) << " ms\n";
  stats_str << "  Maximum: " << doubleToString(stats.maxClickLatency) << " ms\n";
  stats_str << "  Last: " << doubleToString(tracker.getLastClickLatency()) << " ms\n\n";
  
  stats_str << "DISPLAY LATENCY:\n";
  stats_str << "  Count: " << stats.displayCount << "\n";
  stats_str << "  Average: " << doubleToString(stats.avgDisplayLatency) << " ms\n";
  stats_str << "  Minimum: " << doubleToString(stats.minDisplayLatency) << " ms\n";
  stats_str << "  Maximum: " << doubleToString(stats.maxDisplayLatency) << " ms\n";
  stats_str << "  Last: " << doubleToString(tracker.getLastDisplayLatency()) << " ms\n";
  
  return stats_str.str();
}

bool LatencyMetrics::saveToFile(const LatencyTracker& tracker,
                                const char* filename,
                                const char* format)
{
  std::ofstream file(filename);
  if (!file.is_open()) {
    return false;
  }
  
  std::string content;
  
  if (strcmp(format, "json") == 0) {
    content = exportToJSON(tracker);
  } else if (strcmp(format, "csv") == 0) {
    content = exportToCSV(tracker);
  } else if (strcmp(format, "stats") == 0) {
    content = exportStatistics(tracker);
  } else {
    return false;
  }
  
  file << content;
  file.close();
  
  return true;
}
