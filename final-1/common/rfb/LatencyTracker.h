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

// -=- LatencyTracker.h - Tracks click and display latencies

#ifndef __RFB_LATENCYTRACKER_H__
#define __RFB_LATENCYTRACKER_H__

#include <map>
#include <vector>
#include <sys/time.h>
#include <cstddef>
#include <rdr/types.h>
#include <os/Mutex.h>

namespace rfb {

  struct LatencyData {
    rdr::U32 id;
    double latencyMs;
    struct timeval timestamp;
    int eventType; // 0=click, 1=display
    
    LatencyData() : id(0), latencyMs(0.0), eventType(0) {
      timestamp.tv_sec = 0;
      timestamp.tv_usec = 0;
    }
  };

  class LatencyTracker {
  public:
    LatencyTracker();
    ~LatencyTracker();

    // Click latency tracking
    void recordClickStart(rdr::U32 clickId, int x, int y);
    void recordClickComplete(rdr::U32 clickId);
    
    // Display latency tracking
    void recordDisplayStart(rdr::U32 frameId);
    void recordDisplayComplete(rdr::U32 frameId);
    
    // Get latest measurements
    double getLastClickLatency() const;
    double getLastDisplayLatency() const;
    double getAverageClickLatency() const;
    double getAverageDisplayLatency() const;
    
    // Get all recorded latencies
    const std::vector<LatencyData>& getAllLatencies() const;
    
    // Statistics
    struct Statistics {
      double minClickLatency;
      double maxClickLatency;
      double avgClickLatency;
      double minDisplayLatency;
      double maxDisplayLatency;
      double avgDisplayLatency;
      int clickCount;
      int displayCount;
      
      Statistics() : minClickLatency(0), maxClickLatency(0), avgClickLatency(0),
                     minDisplayLatency(0), maxDisplayLatency(0), avgDisplayLatency(0),
                     clickCount(0), displayCount(0) {}
    };
    
    Statistics getStatistics() const;
    
    // Clear old records (keep only recent N)
    void pruneOldRecords(size_t maxRecords = 1000);
    
  private:
    struct ClickEvent {
      struct timeval startTime;
      struct timeval completeTime;
      int x, y;
      bool completed;
      
      ClickEvent() : x(0), y(0), completed(false) {
        startTime.tv_sec = 0;
        startTime.tv_usec = 0;
        completeTime.tv_sec = 0;
        completeTime.tv_usec = 0;
      }
    };
    
    struct DisplayEvent {
      struct timeval startTime;
      struct timeval completeTime;
      bool completed;
      
      DisplayEvent() : completed(false) {
        startTime.tv_sec = 0;
        startTime.tv_usec = 0;
        completeTime.tv_sec = 0;
        completeTime.tv_usec = 0;
      }
    };
    
    double timeDiffMs(const struct timeval& start, const struct timeval& end);
    
    std::map<rdr::U32, ClickEvent> clickEvents;
    std::map<rdr::U32, DisplayEvent> displayEvents;
    std::vector<LatencyData> recordedLatencies;
    
    os::Mutex* mutex;
  };

} // namespace rfb

#endif // __RFB_LATENCYTRACKER_H__
