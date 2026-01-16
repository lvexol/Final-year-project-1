/* Copyright (C) 2024 - Latency Tracking Module
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

// -=- LatencyTracker.cxx - Implementation of latency tracking

#include <rfb/LatencyTracker.h>
#include <rfb/util.h>
#include <rfb/LogWriter.h>
#include <os/Mutex.h>
#include <algorithm>

using namespace rfb;

static rfb::LogWriter vlog("LatencyTracker");

LatencyTracker::LatencyTracker()
{
  mutex = new os::Mutex();
}

LatencyTracker::~LatencyTracker()
{
  delete mutex;
}

double LatencyTracker::timeDiffMs(const struct timeval& start, const struct timeval& end)
{
  long seconds = end.tv_sec - start.tv_sec;
  long microseconds = end.tv_usec - start.tv_usec;
  
  if (microseconds < 0) {
    seconds--;
    microseconds += 1000000;
  }
  
  return (seconds * 1000.0) + (microseconds / 1000.0);
}

void LatencyTracker::recordClickStart(rdr::U32 clickId, int x, int y)
{
  os::AutoMutex a(mutex);
  
  ClickEvent& ce = clickEvents[clickId];
  gettimeofday(&ce.startTime, NULL);
  ce.x = x;
  ce.y = y;
  ce.completed = false;
}

void LatencyTracker::recordClickComplete(rdr::U32 clickId)
{
  os::AutoMutex a(mutex);
  
  std::map<rdr::U32, ClickEvent>::iterator it = clickEvents.find(clickId);
  if (it == clickEvents.end()) {
    vlog.debug("Click event %u not found", clickId);
    return;
  }
  
  gettimeofday(&it->second.completeTime, NULL);
  it->second.completed = true;
  
  double latency = timeDiffMs(it->second.startTime, it->second.completeTime);
  
  LatencyData data;
  data.id = clickId;
  data.latencyMs = latency;
  data.eventType = 0; // Click
  gettimeofday(&data.timestamp, NULL);
  
  recordedLatencies.push_back(data);
  
  vlog.debug("Click %u latency: %.2f ms at (%d, %d)", clickId, latency, it->second.x, it->second.y);
}

void LatencyTracker::recordDisplayStart(rdr::U32 frameId)
{
  os::AutoMutex a(mutex);
  
  DisplayEvent& de = displayEvents[frameId];
  gettimeofday(&de.startTime, NULL);
  de.completed = false;
}

void LatencyTracker::recordDisplayComplete(rdr::U32 frameId)
{
  os::AutoMutex a(mutex);
  
  std::map<rdr::U32, DisplayEvent>::iterator it = displayEvents.find(frameId);
  if (it == displayEvents.end()) {
    vlog.debug("Display event %u not found", frameId);
    return;
  }
  
  gettimeofday(&it->second.completeTime, NULL);
  it->second.completed = true;
  
  double latency = timeDiffMs(it->second.startTime, it->second.completeTime);
  
  LatencyData data;
  data.id = frameId;
  data.latencyMs = latency;
  data.eventType = 1; // Display
  gettimeofday(&data.timestamp, NULL);
  
  recordedLatencies.push_back(data);
  
  vlog.debug("Display %u latency: %.2f ms", frameId, latency);
}

double LatencyTracker::getLastClickLatency() const
{
  os::AutoMutex a(mutex);
  
  // Find last click event
  for (auto it = recordedLatencies.rbegin(); it != recordedLatencies.rend(); ++it) {
    if (it->eventType == 0) {
      return it->latencyMs;
    }
  }
  
  return 0.0;
}

double LatencyTracker::getLastDisplayLatency() const
{
  os::AutoMutex a(mutex);
  
  // Find last display event
  for (auto it = recordedLatencies.rbegin(); it != recordedLatencies.rend(); ++it) {
    if (it->eventType == 1) {
      return it->latencyMs;
    }
  }
  
  return 0.0;
}

double LatencyTracker::getAverageClickLatency() const
{
  os::AutoMutex a(mutex);
  
  double sum = 0.0;
  int count = 0;
  
  for (const auto& data : recordedLatencies) {
    if (data.eventType == 0) {
      sum += data.latencyMs;
      count++;
    }
  }
  
  return count > 0 ? sum / count : 0.0;
}

double LatencyTracker::getAverageDisplayLatency() const
{
  os::AutoMutex a(mutex);
  
  double sum = 0.0;
  int count = 0;
  
  for (const auto& data : recordedLatencies) {
    if (data.eventType == 1) {
      sum += data.latencyMs;
      count++;
    }
  }
  
  return count > 0 ? sum / count : 0.0;
}

const std::vector<LatencyData>& LatencyTracker::getAllLatencies() const
{
  return recordedLatencies;
}

LatencyTracker::Statistics LatencyTracker::getStatistics() const
{
  os::AutoMutex a(mutex);
  
  Statistics stats;
  
  double clickMin = -1, clickMax = -1, clickSum = 0.0;
  double displayMin = -1, displayMax = -1, displaySum = 0.0;
  
  for (const auto& data : recordedLatencies) {
    if (data.eventType == 0) { // Click
      stats.clickCount++;
      clickSum += data.latencyMs;
      if (clickMin < 0 || data.latencyMs < clickMin) clickMin = data.latencyMs;
      if (clickMax < 0 || data.latencyMs > clickMax) clickMax = data.latencyMs;
    } else { // Display
      stats.displayCount++;
      displaySum += data.latencyMs;
      if (displayMin < 0 || data.latencyMs < displayMin) displayMin = data.latencyMs;
      if (displayMax < 0 || data.latencyMs > displayMax) displayMax = data.latencyMs;
    }
  }
  
  if (stats.clickCount > 0) {
    stats.minClickLatency = clickMin;
    stats.maxClickLatency = clickMax;
    stats.avgClickLatency = clickSum / stats.clickCount;
  }
  
  if (stats.displayCount > 0) {
    stats.minDisplayLatency = displayMin;
    stats.maxDisplayLatency = displayMax;
    stats.avgDisplayLatency = displaySum / stats.displayCount;
  }
  
  return stats;
}

void LatencyTracker::pruneOldRecords(size_t maxRecords)
{
  os::AutoMutex a(mutex);
  
  if (recordedLatencies.size() > maxRecords) {
    size_t toRemove = recordedLatencies.size() - maxRecords;
    recordedLatencies.erase(recordedLatencies.begin(), 
                           recordedLatencies.begin() + toRemove);
  }
}
